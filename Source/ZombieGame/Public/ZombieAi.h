// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "CombatInterface.h"
#include "ZombieAi.generated.h"

class UNiagaraSystem;
class AZombieSpawner;
//implement ai reacting to gunshot sounds, decibels could be implemented for hearing range
	//this is where different amunition comes into play for supersonic or subsonic ammunition

	//later on random/procedural effects to the zombies with different effects and skins
	//some that are fresher with more blood? 
	//maybe some with more or less armour which would help with different ammo uses

	//maybe add infection? 



	//put together health system with bleed and gore
//use the players AddXP function whenever you dismembre, kill and maybe bleed time <- its all ready to go just need to just give it an amount

//maybe add more states??
//if the zombies get enraged or something or some have the ability to be faster than others, or even climb 

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
	bool bDisconnected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BleedSeverity;

	FLimbData()
		: MaxHealth(100.f), CurrentHealth(100.f), bDisconnected(false), BleedSeverity(1.0f)
	{
	}

	FLimbData(float InHealth, float bleedSeverity)
		: MaxHealth(InHealth), CurrentHealth(InHealth), bDisconnected(false), BleedSeverity(bleedSeverity)
	{
	}
};

UCLASS()
class ZOMBIEGAME_API AZombieAi : public ACharacter, public ICombatInterface
{
	GENERATED_BODY()

public:
	AZombieAi();

	void ApplyLimbDamage(UPrimitiveComponent* HitComp, float Damage);

	UFUNCTION()
	void DismemberLimb(UPrimitiveComponent* HitComp);

	UBehaviorTree* GetBehaviourTree() const;

	int MeleeAttack_Implementation() override;

	void SetSpawner(AZombieSpawner* inSpawner);

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
	UStaticMeshComponent* TorsoMesh;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* BloodFX;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float damage;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float MaxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, Category = "Zombie Stats")
	TMap<FName, FLimbData> LimbHealthMap;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float bloodQuantity; //in litres

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float bleedOutRate; // how often in seconds should the bleed damage tick off the zombie

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

	bool bIsDead;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	EZombieState CurrentState;

	UFUNCTION(BlueprintPure, Category = "AI")
	EZombieState GetZombieState() const { return CurrentState; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	APawn* TargetPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta=(AllowPrivateAccess="true"))
	UBehaviorTree* Tree;

	//DO NOT FUCKING TOUCH THESE
	UFUNCTION()
	void AttackPlayer(APawn* Player);

	UFUNCTION()
	void HandleSeePlayer(APawn* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnSeePlayer(APawn* Player);

	UFUNCTION()
	void ApplyBleed(float bleedSeverity);

private:
	UFUNCTION()
	void KillZombie();

	UPROPERTY()
	FTimerHandle TimerHandle;

	UPROPERTY()
	AZombieSpawner* Spawner;

	UFUNCTION()
	void DestroyZombie();

	UFUNCTION()
	void SetCurrentHealth(float health);

	UFUNCTION()
	float GetCurrentHealth();

	UFUNCTION()
	void SetCrawlMode();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
