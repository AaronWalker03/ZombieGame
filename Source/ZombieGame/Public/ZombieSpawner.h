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

	int zombiesToSpawn = 0;

	int roundNum = 0;

	void SpawnZombie();
	void StartNewRound();
	void RoundManager();

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AZombieAi> ZombieClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<AActor*> SpawnPoints;

	int spawnIndex = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
