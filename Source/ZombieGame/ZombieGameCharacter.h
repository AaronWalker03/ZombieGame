// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Public/Weapon.h"
#include "PlayerCustomisationStruct.h"
#include "ZombieGameCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

//need to make levelling system, doesnt actually have to unlock anything yet but just have it there saves time later
//^^^already have XP and player level implemented
//can easily make this exponential

//need to make a behaviour tree for animations
//also need to sort out which mesh is seen by the player and which is being sent to server (for others)

UCLASS(abstract)
class AZombieGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

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

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* MouseLookAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AWeapon* EquippedWeapon;

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

	UFUNCTION(BlueprintCallable)
	void SaveCustomisation();

	void LoadCustomisation();

	void AddXP(int amount);

	void Tick(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aiming Bool")
	bool bIsAiming = false;

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


	void SpawnWeapon();

	UFUNCTION(BlueprintCallable)
	void ChangeWeapon(TSubclassOf<AWeapon> NewWeaponClass);

	/** Property replication */
	//void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	void OnFire();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimSequence* aimAnimation;

	
	void StartAim();
	void StopAim();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimSequence* EquipSequence;

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

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

	/** Delay between shots in seconds. Used to control fire rate for your test projectile, but also to prevent an overflow of server functions from binding SpawnProjectile directly to input.*/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float FireRate;

	/** If true, you are in the process of firing projectiles. */
	bool bIsFiringWeapon;

	/** Function for beginning weapon fire.*/
	//UFUNCTION(BlueprintCallable, Category = "Gameplay")
	//void StartFire();

	///** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	//UFUNCTION(BlueprintCallable, Category = "Gameplay")
	//void StopFire();

	///** Server function for spawning projectiles.*/
	//UFUNCTION(Server, Reliable)
	//void HandleFire();

	/** A timer handle used for providing the fire rate delay in-between spawns.*/
	FTimerHandle FiringTimer;

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	private:
		
		class UAIPerceptionStimuliSourceComponent* StimulusSource;

		void SetupStimulusSource();

};

