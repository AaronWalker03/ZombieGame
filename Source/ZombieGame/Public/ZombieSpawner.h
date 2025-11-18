// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawner.generated.h"

UCLASS()
class ZOMBIEGAME_API AZombieSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZombieSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	int aliveZombies = 0;

	void SpawnZombie();

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AZombieAi> ZombieClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
