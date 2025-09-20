// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAi.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AZombieAi::AZombieAi()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    pawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
}

// Called when the game starts or when spawned
void AZombieAi::BeginPlay()
{
	Super::BeginPlay();

   

    pawnSensingComp->SensingInterval = 0.5f;
    pawnSensingComp->bSeePawns = true;

    // Default stats
    health = 100.f;
    damage = 10.f;
    movementSpeed = 150.f;
    visionDetectionAngle = 50.0f;
    visionDetectionRange = 1200.0f;
    pawnSensingComp->bSeePawns = true;
    pawnSensingComp->bOnlySensePlayers = false;

    // Apply movement speed
    GetCharacterMovement()->MaxWalkSpeed = movementSpeed;

    // Apply sensing component settings
    pawnSensingComp->SightRadius = visionDetectionRange;
    pawnSensingComp->SetPeripheralVisionAngle(visionDetectionAngle);
	
    if (pawnSensingComp)
    {
        pawnSensingComp->OnSeePawn.AddDynamic(this, &AZombieAi::HandleSeePlayer);
    }
}

// Called every frame
void AZombieAi::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (pawnSensingComp)
    {
        DrawDebugCone(
            GetWorld(),
            GetActorLocation(),
            GetActorForwardVector(),
            pawnSensingComp->SightRadius,
            FMath::DegreesToRadians(pawnSensingComp->GetPeripheralVisionAngle() / 2.0f), // half-angle
            FMath::DegreesToRadians(pawnSensingComp->GetPeripheralVisionAngle() / 2.0f), // half-angle
            12,
            FColor::Green,
            false,
            0.1f,
            0,
            1.0f
        );
    }
}

void AZombieAi::HandleSeePlayer(APawn* Player)
{
    if (!Player) return;

    // Call Blueprint event if implemented
    OnSeePlayer(Player);

    // Move towards player using AIController
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        AIController->MoveToActor(Player, 50.f); // Stop 50 units away
    }
}