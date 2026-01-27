// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawner.h"
#include "ZombieAi.h"
#include "ZombieSpawnPoint.h"
#include "EngineUtils.h"

//implement zombies dying so rounds actually change

AZombieSpawner::AZombieSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

    AliveZombies = 0;
    NumToSpawn = 6;
    RoundNum = 1;
}

void AZombieSpawner::BeginPlay()
{
	Super::BeginPlay();
	
    GetSpawnPoints();
    GetPlayerPawns();
}

// Called every frame
void AZombieSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    RoundManager();
}

void AZombieSpawner::RoundManager()
{
    if (NumToSpawn && AliveZombies == 0)
    {
        StartNewRound();
        return;
    }

    if (NumToSpawn > 0)
    {
        if (AliveZombies < HordeCap)
        {
            SpawnZombie();
        }
    }
}

void AZombieSpawner::GetSpawnPoints()
{
    for (TActorIterator<AZombieSpawnPoint> it(GetWorld()); it; ++it)
    {
        SpawnPoints.Add(*it);
    }
}

void AZombieSpawner::GetPlayerPawns()
{
    for (FConstPlayerControllerIterator pcIt = GetWorld()->GetPlayerControllerIterator(); pcIt; ++pcIt)
    {
        APlayerController* pc = pcIt->Get();
        if (!pc) continue;

        APawn* pawn = pc->GetPawn();
        if (!pawn) continue;

        PlayerPawns.Add(pawn);
    }
}

AZombieSpawnPoint* AZombieSpawner::GetClosestSpawnPointToPlayer()
{
    AZombieSpawnPoint* closestSpawn = nullptr;
    float closestDistSq = TNumericLimits<float>::Max();

    for (APawn* pawn : PlayerPawns)
    {
        FVector playerLocation = pawn->GetActorLocation();

        for (AZombieSpawnPoint* spawnPoint : SpawnPoints)
        {
            if (!IsValid(spawnPoint)) continue;

            float distSq = FVector::DistSquared(
                spawnPoint->GetActorLocation(),
                playerLocation
            );

            if (distSq < closestDistSq)
            {
                closestDistSq = distSq;
                closestSpawn = spawnPoint;
            }
        }
    }

    return closestSpawn;
}

void AZombieSpawner::StartNewRound()
{
    RoundNum++;

    NumToSpawn = BaseNumOfZombies + (RoundNum * LinearRoundIncrease) + (RoundNum * RoundNum * QuadraticRoundIncrease);

    // clear any dead bodies at this point?
    // idk if we want to go full cod zombies and have powerups - then reset AllowedNumberOfDropsThatCanSpawn

    UE_LOG(LogTemp, Warning, TEXT("Starting round %d with %d zombies"),
        RoundNum, NumToSpawn);
}

void AZombieSpawner::SpawnZombie()
{
    AZombieSpawnPoint* spawnPoint = GetClosestSpawnPointToPlayer();

    FVector Location = spawnPoint->GetActorLocation();

    //Location.Z += 100.f;

    FRotator Rotation = spawnPoint->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn zombie
    GetWorld()->SpawnActor<AZombieAi>(ZombieClass, Location, Rotation, SpawnParams);

    AliveZombies++;
    NumToSpawn--;
}