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
    NumToSpawn = BaseNumOfZombies;
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

void AZombieSpawner::TrySpawnZombies()
{
    if (NumToSpawn <= 0) return;

    if (AliveZombies >= HordeCap) return;

    int32 canSpawn = HordeCap - AliveZombies;
    int32 spawnNow = FMath::Min(canSpawn, NumToSpawn);

    SpawnWave(spawnNow);
}

void AZombieSpawner::SpawnWave(int SpawnCount)
{
    if (PlayerPawns.Num() == 0) return;

    int ZombiesPerPlayer = SpawnCount / PlayerPawns.Num();
    int Remainder = SpawnCount % PlayerPawns.Num();

    for (APawn* player : PlayerPawns)
    {
        int PlayerSpawnCount = ZombiesPerPlayer;

        if (Remainder > 0)
        {
            PlayerSpawnCount++;
            Remainder--;
        }

        for (int32 i = 0; i < PlayerSpawnCount; i++)
        {
            SpawnZombieNearPlayer(player);
            NumToSpawn--;
        }
    }
}

AZombieSpawnPoint* AZombieSpawner::GetClosestSpawnPointToPlayer(APawn* Player)
{
    AZombieSpawnPoint* closestSpawn = nullptr;
    float closestDistSq = TNumericLimits<float>::Max();

    FVector playerLocation = Player->GetActorLocation();

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

    return closestSpawn;
}

void AZombieSpawner::NotifyZombieDied()
{
    AliveZombies--;
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

void AZombieSpawner::SpawnZombieNearPlayer(APawn* Player)
{
    if (!Player) return;

    AZombieSpawnPoint* spawnPoint = GetClosestSpawnPointToPlayer(Player);
    if (!spawnPoint) return;

    FVector location = spawnPoint->GetActorLocation();
    FRotator rotation = spawnPoint->GetActorRotation();

    FActorSpawnParameters spawnParams;
    spawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AZombieAi* zombie = GetWorld()->SpawnActor<AZombieAi>(
        ZombieClass,
        location,
        rotation,
        spawnParams
    );

    if (zombie)
    {
        zombie->SetSpawner(this);
        AliveZombies++;
    }

    //AZombieSpawnPoint* spawnPoint = GetClosestSpawnPointToPlayer();

    //FVector Location = spawnPoint->GetActorLocation();

    ////Location.Z += 100.f;

    //FRotator Rotation = spawnPoint->GetActorRotation();

    //FActorSpawnParameters SpawnParams;
    //SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    //// Spawn zombie
    //GetWorld()->SpawnActor<AZombieAi>(ZombieClass, Location, Rotation, SpawnParams);

    //AliveZombies++;
    //NumToSpawn--;
}