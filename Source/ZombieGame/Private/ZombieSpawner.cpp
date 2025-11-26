// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawner.h"
#include "ZombieAi.h"




//implement zombies dying so rounds actually change

AZombieSpawner::AZombieSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AZombieSpawner::BeginPlay()
{
	Super::BeginPlay();
	
    roundNum = 1;

    zombiesToSpawn = 15;
}

// Called every frame
void AZombieSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    RoundManager();
}

void AZombieSpawner::RoundManager()
{
    if (zombiesToSpawn > 0)
    {
        SpawnZombie();
        zombiesToSpawn--;
        aliveZombies++;
        return;
    }

    if (aliveZombies == 0)
    {
        StartNewRound();
    }
}

void AZombieSpawner::StartNewRound()
{
    roundNum++;

    zombiesToSpawn = 5 + (roundNum - 1) * 2;

    UE_LOG(LogTemp, Warning, TEXT("Starting round %d with %d zombies"),
        roundNum, zombiesToSpawn);
}

void AZombieSpawner::SpawnZombie()
{
    AActor* SpawnPoint = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];

    FVector Location = SpawnPoint->GetActorLocation();

   
    Location.Z += 100.f;  

    FRotator Rotation = SpawnPoint->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn zombie
    GetWorld()->SpawnActor<AZombieAi>(ZombieClass, Location, Rotation, SpawnParams);

    spawnIndex++;
}