// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawner.generated.h"

class AZombieSpawnPoint;

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

	const int HordeCap = 24;	// limit of zombies that can be on the map at any one time

	const int BaseNumOfZombies = 6;

	const int LinearRoundIncrease = 3;

	const float QuadraticRoundIncrease = 0.25f;

	int AliveZombies;

	int NumToSpawn;

	int RoundNum;

	void SpawnZombieNearPlayer(APawn* PlayerPawn);
	void StartNewRound();
	void RoundManager();
	void GetSpawnPoints();
	void GetPlayerPawns();
	void TrySpawnZombies();
	void SpawnWave(int SpawnCount);

	AZombieSpawnPoint* GetClosestSpawnPointToPlayer(APawn* Player);

	void NotifyZombieDied();

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AZombieAi> ZombieClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<AZombieSpawnPoint*> SpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<APawn*> PlayerPawns;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
