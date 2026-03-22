// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Public/Weapon.h"
#include "PlayerCustomisationStruct.h"
#include <GameFramework/SpringArmComponent.h>
#include "ZombieGameCharacter.generated.h"



class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;
struct FInputActionInstance;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

//need to make levelling system, doesnt actually have to unlock anything yet but just have it there saves time later
//^^^already have XP and player level implemented
//can easily make this exponential

//need to make a behaviour tree for animations
//also need to sort out which mesh is seen by the player and which is being sent to server (for others)


//need to get ref to fps mesh
//need to make it so that can have multiple weapons
//make it so that player can pick up weapons
//rework weapon bp to work with new ads system

UCLASS(abstract)
class AZombieGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;


protected:

	//
   // 1. REPLICATED CUSTOMISATION DATA
   //
	UPROPERTY(ReplicatedUsing = OnRep_Customisation)
	FPlayerCustomisationData PlayerCustomisation;

	//
	// 2. CALLED ON CLIENT WHEN PlayerCustomisation UPDATES
	//
	UFUNCTION()
	void OnRep_Customisation();

	//
	// 3. CLIENT  SERVER SENDS ITS CUSTOMISATION
	//
	UFUNCTION(Server, Reliable)
	void Server_SendCustomisation(const FPlayerCustomisationData& Data);

	//
	// 4. APPLY CUSTOMISATION LOCALLY
	//
	void ApplyCustomisation(const FPlayerCustomisationData& Data);

	//
	// 5. LOAD SAVEGAME ON THE LOCAL CLIENT
	//

	void LoadCustomisationFromSave();

	UFUNCTION(Server, Reliable)
	void Server_Fire();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireEffects();

	UFUNCTION(Server, Reliable)
	void Server_SwitchToPrimary();

	UFUNCTION(Server, Reliable)
	void Server_SwitchToSecondary();

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayReload(UAnimMontage* Montage);

	UFUNCTION()
	void OnRep_HeldWeapon();



	USkeletalMeshComponent* FirstPersonMesh;

	void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnMagCheckMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* simpleReload;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* speedReload;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* magCheck;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* sprintingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* fireMode;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* crouchAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SwitchPrimaryAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SwitchSecondaryAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* MouseLookAction;

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	//these are now the default selectors
	/*UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AWeapon* primaryWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AWeapon* secondaryWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AWeapon* heldWeapon;*/

	UPROPERTY(Replicated)
	AWeapon* primaryWeapon;

	UPROPERTY(Replicated)
	AWeapon* secondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_HeldWeapon)
	AWeapon* heldWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultPrimaryWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultSecondaryWeapon;


	//current weapon held

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	//AWeapon* EquippedMelee;

	/** The player's maximum health. This is the highest value of their health can be. This value is a value of the player's health, which starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	int currentXP;
	
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	AZombieGameCharacter();

	void BeginPlay();

	void AttachWeaponToSocket(AWeapon* Weapon, FName SocketName, bool isOnFPMesh);

	void EquipWeapon(AWeapon* WeaponToEquip);

	UFUNCTION(BlueprintCallable)
	void SaveCustomisation();

	void LoadCustomisation();

	void AddXP(int amount);

	void Tick(float DeltaTime);

	bool bIsAiming = false;
	bool bIsReloading = false;
	bool fullAuto = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* FP_Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* MeshRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Offset_Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* Cam_Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mag Check")
	bool bIsMagChecking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mag Check")
	int ammoInMag = 0;

	//change this to a list, so can have specific ammo per mag
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mag Count")
	int magCount = 4;

	UFUNCTION(BlueprintCallable)
	void MagCheck();

	// Movement tuning
	float BaseWalkSpeed = 450.f;

	// Animation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	float WalkAnimAlpha = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	FVector WalkOffset = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	FRotator WalkRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	float ADSAlpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	float ADSInterpSpeed = 12.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	float HipFOV = 90.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	float ADSFOV = 65.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	float HipPPWeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	float ADSPPWeight = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Variables")
	FVector ADSOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ArReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* arFullReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* arMagCheckMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* pistolReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* pistolFullReloadMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* pistolMagCheckMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* arShootMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* pistolShootMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool isHoldingPrimary = true;

	// Footsteps
	float FootstepTimer = 0.f;
	float FootstepInterval = 0.45f;


	float SprintSpeed = 650.f;
	float SpeedInterpRate = 8.f;

	bool bIsFiring = false;

	FTimerHandle AutoFireTimer;

	UPROPERTY(EditDefaultsOnly)
	float FireRate = 0.1f; // 600 RPM approx



	//call save file and get meshes saved

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothes Meshes")
	USkeletalMeshComponent* footWear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothes Meshes")
	USkeletalMeshComponent* legWear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothes Meshes")
	USkeletalMeshComponent* tshirtWear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothes Meshes")
	USkeletalMeshComponent* jacketWear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothes Meshes")
	USkeletalMeshComponent* faceWear;

	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> weaponToSpawn);

	UFUNCTION(BlueprintCallable)
	void ChangeWeapon(TSubclassOf<AWeapon> NewWeaponClass);

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	void UpdateWalkAnimation(float DeltaTime);
	void HandleFootsteps(float DeltaTime);
	void StartSprint();
	void StopSprint();
	void UpdateMovementSpeed(float DeltaTime);

	void UpdateADS(float DeltaTime);

	void OnFire();
	void AutoFire();
	void StopFiring();
	
	void StartAim();
	void StopAim();

	void SwitchFireMode();

	void SimpleReload();
	void SpeedReload();
	
	void SwitchToPrimary();
	void SwitchToSecondary();

	


	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimSequence* EquipSequence;*/



	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
	TSubclassOf<class AZombieGameProjectile> ProjectileClass;

public:

	/** Returns the first person mesh **/
	//USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	private:
		
		class UAIPerceptionStimuliSourceComponent* StimulusSource;

		void SetupStimulusSource();

};

