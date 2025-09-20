// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieAi.generated.h"

UCLASS()
class ZOMBIEGAME_API AZombieAi : public ACharacter
{
	GENERATED_BODY()

	//enum states for actual states the zombie is in then maybe based on the states the animation follows

public:
	// Sets default values for this character's properties
	AZombieAi();

protected:
	// Called when the game starts or when spawned
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


	//DO NOT FUCKING TOUCH THESE
	UFUNCTION()
	void HandleSeePlayer(APawn* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnSeePlayer(APawn* Player);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
