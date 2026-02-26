// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieGameCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <InputAction.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "ZombieGameProjectile.h"
#include "PlayerCustomisationSave.h"
#include "PlayerCustomisationStruct.h"
#include <Kismet/GameplayStatics.h>
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"




DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AZombieGameCharacter


//could add dismemberent for players? if you get bitten in multiplayer to stay alive longer chop a limb off? 
//would potentially require new animations tho

//pressing r once slowly reloads while keeping the mag
//double tap r quick reload but lose the mag, conveniant if you fully empty

//make multi weapon slot system, melee, pistol, 2 big guns
//tie this into customisation
//then change the animation bp depending on which weapon holding

//for customisation menu make it like tarkov so we dont have custimsation issues
//just a simple send and receive and assign
//save file needs to be different because when adding xp manually saving to file each time but doesnt do the rest for some reason
//otherwise make it when you die or extract it saves everything
//change this so that it applies the other players stuff on client side on THEIR character
//curently does it for the client
//also need to make it so that weapons are applied to the third person meshes for other players to see

//use current weapons then tell it to only equip to third person if isnt client?
//for first person shadows, get the mesh of weapon and attach it to third person mesh

//add vaulting instead of jumping?

//add leaning

//make a function so adjust ads and recoil for any equiped weapon


//naming conventions for send/receive save files


//might need make a method to attach left arm to weapon depending on grip (wihout affecting current attachment on right hand)
//do this through anim bp?

AZombieGameCharacter::AZombieGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(36.f, 96.0f);

	FP_Root = CreateDefaultSubobject<USceneComponent>(TEXT("FP_Root"));
	FP_Root->SetupAttachment(GetCapsuleComponent());

	MeshRoot = CreateDefaultSubobject<USpringArmComponent>(TEXT("MeshRoot"));
	MeshRoot->SetupAttachment(FP_Root);
	MeshRoot->TargetArmLength = 0.f;
	MeshRoot->bUsePawnControlRotation = true;
	MeshRoot->bDoCollisionTest = false;

	Offset_Root = CreateDefaultSubobject<USceneComponent>(TEXT("Offset_Root"));
	Offset_Root->SetupAttachment(MeshRoot);

	Cam_Root = CreateDefaultSubobject<USpringArmComponent>(TEXT("Cam_Root"));
	Cam_Root->SetupAttachment(FP_Root);
	Cam_Root->TargetArmLength = 0.f;
	Cam_Root->bUsePawnControlRotation = true;
	Cam_Root->bDoCollisionTest = false;

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(Cam_Root);
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 90.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetMesh()->HideBoneByName(TEXT("upperarm_l"), EPhysBodyOp::PBO_None);
	GetMesh()->HideBoneByName(TEXT("upperarm_r"), EPhysBodyOp::PBO_None);

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	ProjectileClass = AZombieGameProjectile::StaticClass();

	footWear = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("footWear"));
	footWear->SetupAttachment(GetMesh());

	legWear = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("legWear"));
	legWear->SetupAttachment(GetMesh());

	tshirtWear = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("tshirtWear"));
	tshirtWear->SetupAttachment(GetMesh());

	jacketWear = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("jacketWear"));
	jacketWear->SetupAttachment(GetMesh());

	faceWear = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("faceWear"));
	faceWear->SetupAttachment(GetMesh());	

	bReplicates = true;

	SetupStimulusSource();
}

void AZombieGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	//can probably get changed and hierarchy reset?
	FirstPersonMesh = Cast<USkeletalMeshComponent>(
		GetDefaultSubobjectByName(TEXT("FirstPersonMesh"))
	);

	//LoadCustomisation();

	FString XPMessage = FString::Printf(TEXT("You now have %d XP"), currentXP);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, XPMessage);

	primaryWeapon = SpawnWeapon(DefaultPrimaryWeapon);
	EquipWeapon(primaryWeapon);
	heldWeapon = primaryWeapon;

	secondaryWeapon = SpawnWeapon(DefaultSecondaryWeapon);
	EquipWeapon(secondaryWeapon);

	GetMesh()->HideBoneByName(TEXT("upperarm_l"), EPhysBodyOp::PBO_None);
	GetMesh()->HideBoneByName(TEXT("upperarm_r"), EPhysBodyOp::PBO_None);
}

void AZombieGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	HandleFootsteps(DeltaTime);
	UpdateMovementSpeed(DeltaTime);	
	UpdateWalkAnimation(DeltaTime);
	UpdateADS(DeltaTime);
	

	//change recoil to a function
	if (heldWeapon)
	{
		float pitchStep = heldWeapon->recoilPitch * DeltaTime * 20.0f;
		float yawStep = heldWeapon->recoilYaw * DeltaTime * 20.0f;

		AddControllerPitchInput(-pitchStep);
		AddControllerYawInput(yawStep);

		heldWeapon->recoilPitch -= pitchStep;
		heldWeapon->recoilYaw -= yawStep;

		heldWeapon->recoilPitch = FMath::FInterpTo(
			heldWeapon->recoilPitch, 0.f, DeltaTime,
			heldWeapon->recoilRecoverySpeed);

		heldWeapon->recoilYaw = FMath::FInterpTo(
			heldWeapon->recoilYaw, 0.f, DeltaTime,
			heldWeapon->recoilRecoverySpeed);
	}
	 
}

void AZombieGameCharacter::SaveCustomisation()
{
	UPlayerCustomisationSave* SaveObj =
		Cast<UPlayerCustomisationSave>(UGameplayStatics::CreateSaveGameObject(UPlayerCustomisationSave::StaticClass()));

	SaveObj->footWearMesh = footWear->GetSkeletalMeshAsset();
	SaveObj->legWearMesh = legWear->GetSkeletalMeshAsset();
	SaveObj->tshirtWearMesh = tshirtWear->GetSkeletalMeshAsset();
	SaveObj->jacketWearMesh = jacketWear->GetSkeletalMeshAsset();
	SaveObj->faceWearMesh = faceWear->GetSkeletalMeshAsset();

	//change for different weapons
	SaveObj->weaponClass = primaryWeapon->GetClass();

	UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("PlayerCustomisation"), 0);
}

void AZombieGameCharacter::LoadCustomisation()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("PlayerCustomisation"), 0))
	{
		UPlayerCustomisationSave* SaveObj =
			Cast<UPlayerCustomisationSave>(UGameplayStatics::LoadGameFromSlot(TEXT("PlayerCustomisation"), 0));

		if (!SaveObj) return;

		footWear->SetSkeletalMesh(SaveObj->footWearMesh.LoadSynchronous());
		legWear->SetSkeletalMesh(SaveObj->legWearMesh.LoadSynchronous());
		tshirtWear->SetSkeletalMesh(SaveObj->tshirtWearMesh.LoadSynchronous());
		jacketWear->SetSkeletalMesh(SaveObj->jacketWearMesh.LoadSynchronous());
		faceWear->SetSkeletalMesh(SaveObj->faceWearMesh.LoadSynchronous());
		DefaultPrimaryWeapon = SaveObj->weaponClass.Get();
		currentXP = SaveObj->currentXP;
	}
}

void AZombieGameCharacter::LoadCustomisationFromSave()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("PlayerSave"), 0))
	{
		UPlayerCustomisationSave* Save =
			Cast<UPlayerCustomisationSave>(UGameplayStatics::LoadGameFromSlot(TEXT("PlayerSave"), 0));

		if (Save)
		{
			PlayerCustomisation.footWearMesh = Save->footWearMesh;
			PlayerCustomisation.legWearMesh = Save->legWearMesh;
			PlayerCustomisation.tshirtWearMesh = Save->tshirtWearMesh;
			PlayerCustomisation.jacketWearMesh = Save->jacketWearMesh;
			PlayerCustomisation.faceWearMesh = Save->faceWearMesh;

			PlayerCustomisation.weaponClass = Save->weaponClass;

			PlayerCustomisation.playerLevel = Save->playerLevel;
			PlayerCustomisation.currentXP = Save->currentXP;
		}
	}
}

void AZombieGameCharacter::Server_SendCustomisation_Implementation(const FPlayerCustomisationData& Data)
{
	PlayerCustomisation = Data;

	// Server also applies it so it replicates to others
	ApplyCustomisation(PlayerCustomisation);
}

void AZombieGameCharacter::OnRep_Customisation()
{
	
	ApplyCustomisation(PlayerCustomisation);
}

void AZombieGameCharacter::ApplyCustomisation(const FPlayerCustomisationData& Data)
{
	// Ensure the character's mesh exists
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
				FString::Printf(TEXT("[%s] ApplyCustomisation: Mesh component is null!"), *GetName()));
		}
		return;
	}

	// Safely apply each wearable mesh only if the component exists
	if (footWear)
	{
		footWear->SetSkeletalMesh(Data.footWearMesh.IsValid() ? Data.footWearMesh.Get() : nullptr);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[%s] footWear component is null!"), *GetName()));
	}

	if (legWear)
	{
		legWear->SetSkeletalMesh(Data.legWearMesh.IsValid() ? Data.legWearMesh.Get() : nullptr);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[%s] legWear component is null!"), *GetName()));
	}

	if (tshirtWear)
	{
		tshirtWear->SetSkeletalMesh(Data.tshirtWearMesh.IsValid() ? Data.tshirtWearMesh.Get() : nullptr);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[%s] tshirtWear component is null!"), *GetName()));
	}

	if (jacketWear)
	{
		jacketWear->SetSkeletalMesh(Data.jacketWearMesh.IsValid() ? Data.jacketWearMesh.Get() : nullptr);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[%s] jacketWear component is null!"), *GetName()));
	}

	if (faceWear)
	{
		faceWear->SetSkeletalMesh(Data.faceWearMesh.IsValid() ? Data.faceWearMesh.Get() : nullptr);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[%s] faceWear component is null!"), *GetName()));
	}

	// Spawn the weapon only if the class is valid
	if (Data.weaponClass.IsValid() && GetWorld())
	{
		AWeapon* WeaponToSpawn = GetWorld()->SpawnActor<AWeapon>(Data.weaponClass.Get());
		if (!WeaponToSpawn && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
				FString::Printf(TEXT("[%s] Failed to spawn weapon!"), *GetName()));
		}
	}
	else if (!Data.weaponClass.IsValid() && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[%s] Weapon class is invalid!"), *GetName()));
	}

	// Debug message for successful application
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
			FString::Printf(TEXT("Customisation applied safely for %s"), *GetName()));
	}
}

void AZombieGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

//	DOREPLIFETIME(AZombieGameCharacter, PlayerCustomisation);
	DOREPLIFETIME(AZombieGameCharacter, CurrentHealth);
	//DOREPLIFETIME(AZombieGameCharacter, bIsSprinting);


	//player shoots and sends
	//apply impacts too
	//later different materials get added

}

void AZombieGameCharacter::AddXP(int amount)
{
	currentXP = currentXP + amount;

	UPlayerCustomisationSave* SaveObj =
		Cast<UPlayerCustomisationSave>(UGameplayStatics::CreateSaveGameObject(UPlayerCustomisationSave::StaticClass()));

	SaveObj->currentXP = currentXP;

	UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("PlayerCustomisation"), 0);
}



AWeapon* AZombieGameCharacter::SpawnWeapon(TSubclassOf<AWeapon> weaponToSpawn)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	AWeapon* weapon = GetWorld()->SpawnActor<AWeapon>(
		weaponToSpawn,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	weapon->mesh->SetCastShadow(false);

	return weapon;
}


//gona have to change animation stuff in anim bp
void AZombieGameCharacter::AttachWeaponToSocket(AWeapon* Weapon, FName SocketName)
{
	Weapon->AttachToComponent(
		FirstPersonMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);
}

//if no main gun only spawn whatevers available
void AZombieGameCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (heldWeapon)
	{
		switch (heldWeapon->HolsterType)
		{
		case EWeaponHolsterType::Primary:
			AttachWeaponToSocket(heldWeapon, "BackSocket");
			isHoldingPrimary = false;
			break;

		case EWeaponHolsterType::Secondary:
			AttachWeaponToSocket(heldWeapon, "HipSocket");
			isHoldingPrimary = true;
			break;
		}
	}

	// Equip new weapon
	heldWeapon = WeaponToEquip;
	AttachWeaponToSocket(heldWeapon, "WeaponSocket");

	// Update ADS
	ADSOffset = heldWeapon->weaponADSOffset;
}

void AZombieGameCharacter::SwitchToPrimary()
{
	EquipWeapon(primaryWeapon);
}

void AZombieGameCharacter::SwitchToSecondary()
{
	EquipWeapon(secondaryWeapon);
}

//not deleting old mesh
//make it so that it can tell which weapon its destroying instead of hardcoded?
void AZombieGameCharacter::ChangeWeapon(TSubclassOf<AWeapon> NewWeaponClass)
{
	AWeapon* NewWeapon = SpawnWeapon(NewWeaponClass);

	primaryWeapon->Destroy();

	switch (NewWeapon->HolsterType)
	{
	case EWeaponHolsterType::Primary:
		primaryWeapon = NewWeapon;
		break;

	case EWeaponHolsterType::Secondary:
		secondaryWeapon = NewWeapon;
		break;
	}

	EquipWeapon(NewWeapon);
}

//swap weapon once spawned

void AZombieGameCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines.
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

void AZombieGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AZombieGameCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AZombieGameCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZombieGameCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AZombieGameCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AZombieGameCharacter::LookInput);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AZombieGameCharacter::StartAim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AZombieGameCharacter::StopAim);

		EnhancedInputComponent->BindAction(simpleReload, ETriggerEvent::Triggered, this, &AZombieGameCharacter::SimpleReload);
		EnhancedInputComponent->BindAction(magCheck, ETriggerEvent::Triggered, this, &AZombieGameCharacter::MagCheck);
	/*	EnhancedInputComponent->BindAction(simpleReload, ETriggerEvent::Started, this, &AZombieGameCharacter::ReloadHoldStarted);
		EnhancedInputComponent->BindAction(simpleReload, ETriggerEvent::Completed, this, &AZombieGameCharacter::OnReloadKeyReleased);*/
		//EnhancedInputComponent->BindAction(speedReload, ETriggerEvent::Triggered, this, &AZombieGameCharacter::SpeedReload);
		

		EnhancedInputComponent->BindAction(sprintingAction, ETriggerEvent::Started, this, &AZombieGameCharacter::StartSprint);
		EnhancedInputComponent->BindAction(sprintingAction, ETriggerEvent::Completed, this, &AZombieGameCharacter::StopSprint);

		EnhancedInputComponent->BindAction(SwitchPrimaryAction, ETriggerEvent::Completed, this, &AZombieGameCharacter::SwitchToPrimary);
		EnhancedInputComponent->BindAction(SwitchSecondaryAction, ETriggerEvent::Completed, this, &AZombieGameCharacter::SwitchToSecondary);

		//EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AZombieGameCharacter::OnFire);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Handle firing projectiles
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AZombieGameCharacter::OnFire);
}

void AZombieGameCharacter::SetupStimulusSource()
{
	StimulusSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulus"));

	if (StimulusSource)
	{
		StimulusSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
		StimulusSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());
		StimulusSource->RegisterWithPerceptionSystem();
	}
}

//WEAPON ACTIONS
//add networking to this
void AZombieGameCharacter::OnFire()
{
	if (!bIsReloading && !bIsMagChecking)
	{
		heldWeapon->Shoot();
	}
}

void AZombieGameCharacter::StartAim()
{
	if (!bIsReloading && !bIsMagChecking)
	{
		bIsAiming = true;
	}
}

void AZombieGameCharacter::StopAim()
{
	bIsAiming = false;
}

void AZombieGameCharacter::UpdateADS(float DeltaTime)
{
	float TargetAlpha = bIsAiming ? 1.0f : 0.0f;

	// Always interpolate — no snapping
	ADSAlpha = FMath::FInterpTo(ADSAlpha, TargetAlpha, DeltaTime, ADSInterpSpeed);

	// Camera FOV
	const float NewFOV = FMath::Lerp(HipFOV, ADSFOV, ADSAlpha);
	FirstPersonCameraComponent->SetFieldOfView(NewFOV);

	// Post process
	const float NewPPWeight = FMath::Lerp(HipPPWeight, ADSPPWeight, ADSAlpha);
	FirstPersonCameraComponent->PostProcessBlendWeight = NewPPWeight;
}

void AZombieGameCharacter::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsReloading = false;
}

void AZombieGameCharacter::OnMagCheckMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsMagChecking = false;
}

void AZombieGameCharacter::SimpleReload()
{
	//add a reload time float so that shooting and other stuff is stopped from running
	//use player to set amount of mags so that it can be used with UI


	//implement different reloads based on primary or secondary

	if (bIsReloading || bIsMagChecking)
	{
		return;
	}

	bIsReloading = true;

	if (heldWeapon->bBoltLockedOpen)
	{
		if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
		{
			AnimInstance->Montage_Play(fullReloadMontage, 1.2f);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AZombieGameCharacter::OnReloadMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, fullReloadMontage);
		}
	}
	else
	{
		if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
		{
			AnimInstance->Montage_Play(ReloadMontage, 1.2f);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AZombieGameCharacter::OnReloadMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, ReloadMontage);
		}
	}

	
	heldWeapon->Reload();
	//keep mag so more ammo can be put into it later

}

//do the same as simple reload but faster and lose mag
void AZombieGameCharacter::SpeedReload()
{
	//if lose the mag you lost it for the rest of the game i guess?
	magCount = magCount - 1;
	bIsReloading = true;
}

void AZombieGameCharacter::MagCheck()
{
	//show how many mags are left too?

	if (bIsReloading || bIsMagChecking)
	{
		return;
	}

	bIsMagChecking = true;

	ammoInMag = heldWeapon->MagCheck();
	UE_LOG(LogTemp, Warning, TEXT("MagCheck started. Ammo in mag: %d"), ammoInMag);

	if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
	{
		if (magCheckMontage)
		{
			AnimInstance->Montage_Play(magCheckMontage, 1.0f);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AZombieGameCharacter::OnMagCheckMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, magCheckMontage);
		}
	}
}


//HEALTH
void AZombieGameCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AZombieGameCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}

void AZombieGameCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}



//MOVEMENT
void AZombieGameCharacter::UpdateMovementSpeed(float DeltaTime)
{
	float TargetSpeed = bIsSprinting ? SprintSpeed : BaseWalkSpeed;
	float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;

	float NewSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, SpeedInterpRate);

	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void AZombieGameCharacter::StartSprint()
{
	bIsSprinting = true;
	bIsAiming = false;
}

void AZombieGameCharacter::StopSprint()
{
	bIsSprinting = false;
}

void AZombieGameCharacter::HandleFootsteps(float DeltaTime)
{
	if (GetVelocity().Size2D() < 10.f) return;

	FootstepTimer += DeltaTime;

	if (FootstepTimer >= FootstepInterval)
	{
		FootstepTimer = 0.f;

		float Pitch = bIsSprinting ? 1.0f : 0.8f;
		float Volume = bIsSprinting ? 1.0f : 0.2f;

		/*UGameplayStatics::PlaySoundAtLocation(
			this,
			FootstepSound,
			GetActorLocation(),
			Volume,
			Pitch
		);*/
	}
}

void AZombieGameCharacter::UpdateWalkAnimation(float DeltaTime)
{
	float Speed = GetVelocity().Size2D();
	bool bIsMoving = Speed > 10.f;

	// Smooth alpha (optional, only if needed for blending)
	float TargetAlpha = bIsMoving ? 1.f : 0.f;
	WalkAnimAlpha = FMath::FInterpTo(WalkAnimAlpha, TargetAlpha, DeltaTime, 8.f);

	// Offsets
	FVector TargetOffset = bIsMoving
		? FVector(0.3f, 0.f, -0.7f)
		: FVector::ZeroVector;

	WalkOffset = FMath::VInterpTo(WalkOffset, TargetOffset, DeltaTime, 6.f);

	// Rotation
	FRotator TargetRotation = bIsMoving
		? FRotator(1.55f, 0.f, 0.f)
		: FRotator::ZeroRotator;

	WalkRotation = FMath::RInterpTo(WalkRotation, TargetRotation, DeltaTime, 6.f);
}


void AZombieGameCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AZombieGameCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

//Assuming this if for server stuff??
void AZombieGameCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AZombieGameCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AZombieGameCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AZombieGameCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

