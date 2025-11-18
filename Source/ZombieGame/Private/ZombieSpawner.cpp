// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawner.h"
#include "ZombieAi.h"

// Sets default values
AZombieSpawner::AZombieSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AZombieSpawner::BeginPlay()
{
	Super::BeginPlay();
	
    
}

// Called every frame
void AZombieSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    SpawnZombie();
}

void AZombieSpawner::SpawnZombie()
{
    FVector SpawnLocation = FVector(0, 0, 0);
    FRotator SpawnRotation = FRotator(0, 0, 0);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn zombie
    AZombieAi* Zombie = GetWorld()->SpawnActor<AZombieAi>(
        ZombieClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );
}