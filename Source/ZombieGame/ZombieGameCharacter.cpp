// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieGameCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "ZombieGameProjectile.h"
#include "PlayerCustomisationSave.h"
#include "PlayerCustomisationStruct.h"
#include <Kismet/GameplayStatics.h>
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AZombieGameCharacter


//could add dismemberent for players? if you get bitten in multiplayer to stay alive longer chop a limb off? 
//would potentially require new animations tho


AZombieGameCharacter::AZombieGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	ProjectileClass = AZombieGameProjectile::StaticClass();

	//add a value in weapon class for how fast it can shoot (each gun gona be different)
	FireRate = 0.25f;
	bIsFiringWeapon = false;

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

	if (IsLocallyControlled())
	{
		LoadCustomisationFromSave();
		Server_SendCustomisation(PlayerCustomisation);
	}

	LoadCustomisation();

	FString XPMessage = FString::Printf(TEXT("You now have %d XP"), currentXP);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, XPMessage);

	SpawnWeapon();

}

void AZombieGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EquippedWeapon)
	{
		// Apply recoil to pitch
		AddControllerPitchInput(-EquippedWeapon->recoilPitch * DeltaTime * 5.0f);

		// Apply recoil to yaw
		AddControllerYawInput(EquippedWeapon->recoilYaw * DeltaTime * 10.0f);

		// Recover over time
		EquippedWeapon->recoilPitch = FMath::FInterpTo(EquippedWeapon->recoilPitch, 0.f, DeltaTime, EquippedWeapon->recoilRecoverySpeed);
		EquippedWeapon->recoilYaw = FMath::FInterpTo(EquippedWeapon->recoilYaw, 0.f, DeltaTime, EquippedWeapon->recoilRecoverySpeed);
	}

}

//save file needs to be different because when adding xp manually saving to file each time but doesnt do the rest for some reason
//otherwise make it when you die or extract it saves everything

void AZombieGameCharacter::SaveCustomisation()
{
	UPlayerCustomisationSave* SaveObj =
		Cast<UPlayerCustomisationSave>(UGameplayStatics::CreateSaveGameObject(UPlayerCustomisationSave::StaticClass()));

	SaveObj->footWearMesh = footWear->GetSkeletalMeshAsset();
	SaveObj->legWearMesh = legWear->GetSkeletalMeshAsset();
	SaveObj->tshirtWearMesh = tshirtWear->GetSkeletalMeshAsset();
	SaveObj->jacketWearMesh = jacketWear->GetSkeletalMeshAsset();
	SaveObj->faceWearMesh = faceWear->GetSkeletalMeshAsset();
	SaveObj->weaponClass = EquippedWeapon->GetClass();

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
		DefaultWeaponClass = SaveObj->weaponClass.Get();
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

	DOREPLIFETIME(AZombieGameCharacter, PlayerCustomisation);
	DOREPLIFETIME(AZombieGameCharacter, CurrentHealth);
}

void AZombieGameCharacter::AddXP(int amount)
{
	currentXP = currentXP + amount;

	UPlayerCustomisationSave* SaveObj =
		Cast<UPlayerCustomisationSave>(UGameplayStatics::CreateSaveGameObject(UPlayerCustomisationSave::StaticClass()));

	SaveObj->currentXP = currentXP;

	UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("PlayerCustomisation"), 0);
}

void AZombieGameCharacter::SpawnWeapon()
{
	// maybe make it so that the arm attaches to the front grip too?? not sure if sockets would allow that tho


	if (!EquippedWeapon && DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		// Spawn the new weapon
		AWeapon* MyWeapon = GetWorld()->SpawnActor<AWeapon>(
			DefaultWeaponClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (MyWeapon && FirstPersonMesh)
		{
			MyWeapon->AttachToComponent(
				FirstPersonMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				FName("HandGrip_R")
			);

			EquippedWeapon = MyWeapon;
		}
	}
}

void AZombieGameCharacter::ChangeWeapon(TSubclassOf<AWeapon> NewWeaponClass)
{
	if (NewWeaponClass != DefaultWeaponClass)
	{
		DefaultWeaponClass = NewWeaponClass;

		if (EquippedWeapon && EquippedWeapon->GetClass() != DefaultWeaponClass)
		{
			// Destroy current weapon
			EquippedWeapon->Destroy();
			EquippedWeapon = nullptr;
		}

		SpawnWeapon();
	}
}

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

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AZombieGameCharacter::OnAim);
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
		StimulusSource->RegisterWithPerceptionSystem();
	}
}

//logic that gets sent over to weapon class
void AZombieGameCharacter::OnFire()
{
	EquippedWeapon->Shoot();
}

void AZombieGameCharacter::OnAim()
{
	EquippedWeapon->Aim();
}

//this no use anymore 
//void AZombieGameCharacter::StartFire()
//{
//	if (!bIsFiringWeapon)
//	{
//		bIsFiringWeapon = true;
//		UWorld* World = GetWorld();
//		World->GetTimerManager().SetTimer(FiringTimer, this, &AZombieGameCharacter::StopFire, FireRate, false);
//		HandleFire();
//	}
//}
//
//void AZombieGameCharacter::StopFire()
//{
//	bIsFiringWeapon = false;
//}
//
//void AZombieGameCharacter::HandleFire_Implementation()
//{
//	FVector spawnLocation = GetActorLocation() + (GetActorRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
//	FRotator spawnRotation = GetActorRotation();
//
//	FActorSpawnParameters spawnParameters;
//	spawnParameters.Instigator = GetInstigator();
//	spawnParameters.Owner = this;
//
//	AZombieGameProjectile* spawnedProjectile = GetWorld()->SpawnActor<AZombieGameProjectile>(spawnLocation, spawnRotation, spawnParameters);
//}

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

void AZombieGameCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}
