// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieAi.generated.h"

UENUM(BlueprintType)
enum class EZombieState : uint8
{
	ZS_Idle      UMETA(DisplayName = "Idle"),
	ZS_Attack    UMETA(DisplayName = "Attack"),
	ZS_Wandering UMETA(DisplayName = "Wandering")
};

UCLASS()
class ZOMBIEGAME_API AZombieAi : public ACharacter
{
	GENERATED_BODY()

	//implement ai reacting to gunshot sounds, decibels could be implemented for hearing range
	//this is where different amunition comes into play for supersonic or subsonic ammunition

public:
	AZombieAi();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float damage;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie Stats")
	float health;

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
