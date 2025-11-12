// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieAi.generated.h"

//implement ai reacting to gunshot sounds, decibels could be implemented for hearing range
	//this is where different amunition comes into play for supersonic or subsonic ammunition

	//later on random/procedural effects to the zombies with different effects and skins
	//some that are fresher with more blood? 
	//maybe some with more or less armour which would help with different ammo uses

	//maybe add infection? 



	//implement damage and body part damage

UENUM(BlueprintType)
enum class EZombieState : uint8
{
	ZS_Idle      UMETA(DisplayName = "Idle"),
	ZS_Attack    UMETA(DisplayName = "Attack"),
	ZS_Wandering UMETA(DisplayName = "Wandering")
};

USTRUCT(BlueprintType)
struct FLimbData
{
	GENERATED_BODY()

	//add position and socket later on jsut so its easier and cleaner

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBleeding;

	FLimbData()
		: MaxHealth(100.f), CurrentHealth(100.f), bBleeding(false)
	{
	}

	FLimbData(float InHealth)
		: MaxHealth(InHealth), CurrentHealth(InHealth), bBleeding(false)
	{
	}
};

UCLASS()
class ZOMBIEGAME_API AZombieAi : public ACharacter
{
	GENERATED_BODY()

public:
	AZombieAi();

	void ApplyLimbDamage(UPrimitiveComponent* HitComp, float Damage);

	UFUNCTION()
	void DismemberLimb(UPrimitiveComponent* HitComp);

protected:
	virtual void BeginPlay() override;

	void SetBodyparts();

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* HeadMesh;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* LUpArm;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* LForearm;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* LHand;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* RUpArm;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* RForearm;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* RHand;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* LThigh;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* LCalf;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* LFoot;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* RThigh;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* RCalf;

	UPROPERTY(VisibleAnywhere, Category = "Body Parts")
	UStaticMeshComponent* RFoot;

	 UPROPERTY(EditDefaultsOnly, Category="Effects")
    UParticleSystem* BloodFX;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float damage;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float health;

	UPROPERTY(EditAnywhere, Category = "Zombie Stats")
	TMap<FName, FLimbData> LimbHealthMap;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float bloodAmount; //in litres?

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float movementSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float audioDetectionRange;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float visionDetectionAngle;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float visionDetectionRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UPawnSensingComponent* pawnSensingComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	EZombieState CurrentState;

	UFUNCTION(BlueprintPure, Category = "AI")
	EZombieState GetZombieState() const { return CurrentState; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	APawn* TargetPlayer;

	//DO NOT FUCKING TOUCH THESE
	UFUNCTION()
	void AttackPlayer(APawn* Player);

	UFUNCTION()
	void HandleSeePlayer(APawn* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnSeePlayer(APawn* Player);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
