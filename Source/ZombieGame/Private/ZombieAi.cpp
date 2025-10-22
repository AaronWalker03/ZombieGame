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

    CurrentState = EZombieState::ZS_Idle;
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

    // lastKnownPlayerLocation = TargetPlayer->GetActorLocation

    switch (CurrentState)
    {
    case EZombieState::ZS_Idle:
        // Maybe look around or play idle animation
        break;

    case EZombieState::ZS_Wandering:
        // Move around randomly

        //Could make it so that it keeps going to the last known location of the player
        //roam around for a bit then to the original spawn location?

       // AiController->MoveTo = lastKnownPlayerLocation

        //once at location roam around like a fent addict
      

        break;

    case EZombieState::ZS_Attack:
        if (TargetPlayer)
        {
            float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

            // If too far, stop chasing
            if (DistanceToPlayer > 1500.0f) // you can tweak this range
            {
                TargetPlayer = nullptr;
                CurrentState = EZombieState::ZS_Wandering;

                AAIController* AIController = Cast<AAIController>(GetController());
                if (AIController)
                {
                    AIController->StopMovement();
                }
            }
        }
        break;
    }

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

void AZombieAi::AttackPlayer(APawn* Player)
{
    // Move towards player using AIController
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        AIController->MoveToActor(Player, 50.f); 
    }
}

void AZombieAi::HandleSeePlayer(APawn* Player)
{
    if (!Player) return;

    TargetPlayer = Player;
    OnSeePlayer(Player);
    AttackPlayer(Player);
    CurrentState = EZombieState::ZS_Attack;
}