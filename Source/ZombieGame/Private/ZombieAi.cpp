// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAi.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "ZombieGameCharacter.h"

// Sets default values
AZombieAi::AZombieAi()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    pawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

    health = 100.0f;

    CurrentState = EZombieState::ZS_Idle;

    SetBodyparts();
}

void AZombieAi::SetBodyparts()
{
    HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
    HeadMesh->SetupAttachment(GetMesh(), FName("head")); // attach to head socket

    LUpArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LUpArm"));
    LUpArm->SetupAttachment(GetMesh(), FName("upperarm_l"));

    RForearm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RForearm"));
    RForearm->SetupAttachment(GetMesh(), FName("lowerarm_r"));

    LHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LHand"));
    LHand->SetupAttachment(GetMesh(), FName("hand_l"));

    RUpArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RUpArm"));
    RUpArm->SetupAttachment(GetMesh(), FName("upperarm_r"));

    LForearm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LForearm"));
    LForearm->SetupAttachment(GetMesh(), FName("lowerarm_l"));

    RHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RHand"));
    RHand->SetupAttachment(GetMesh(), FName("hand_r"));

    LThigh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LThigh"));
    LThigh->SetupAttachment(GetMesh(), FName("thigh_l"));

    LCalf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LCalf"));
    LCalf->SetupAttachment(GetMesh(), FName("calf_l"));

    LFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LFoot"));
    LFoot->SetupAttachment(GetMesh(), FName("foot_l"));

    RThigh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RThigh"));
    RThigh->SetupAttachment(GetMesh(), FName("thigh_r"));

    RCalf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RCalf"));
    RCalf->SetupAttachment(GetMesh(), FName("calf_r"));

    RFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RFoot"));
    RFoot->SetupAttachment(GetMesh(), FName("foot_r"));

    GetMesh()->SetVisibility(false);

    // Load a basic cube mesh from the engine content
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

    HeadMesh->SetStaticMesh(CubeAsset.Object);
    HeadMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));  // Scale down

    LUpArm->SetStaticMesh(CubeAsset.Object);
    LUpArm->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    LForearm->SetStaticMesh(CubeAsset.Object);
    LForearm->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    LHand->SetStaticMesh(CubeAsset.Object);
    LHand->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    RUpArm->SetStaticMesh(CubeAsset.Object);
    RUpArm->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    RForearm->SetStaticMesh(CubeAsset.Object);
    RForearm->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    RHand->SetStaticMesh(CubeAsset.Object);
    RHand->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    LThigh->SetStaticMesh(CubeAsset.Object);
    LThigh->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    LCalf->SetStaticMesh(CubeAsset.Object);
    LCalf->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    LFoot->SetStaticMesh(CubeAsset.Object);
    LFoot->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    RThigh->SetStaticMesh(CubeAsset.Object);
    RThigh->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    RCalf->SetStaticMesh(CubeAsset.Object);
    RCalf->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    RFoot->SetStaticMesh(CubeAsset.Object);
    RFoot->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

    LimbHealthMap.Add("Head", FLimbData(35.f));

    // Arms: low importance
    LimbHealthMap.Add("LUpArm", FLimbData(15.f));
    LimbHealthMap.Add("LForearm", FLimbData(10.f));
    LimbHealthMap.Add("LHand", FLimbData(5.f));
    LimbHealthMap.Add("RUpArm", FLimbData(15.f));
    LimbHealthMap.Add("RForearm", FLimbData(10.f));
    LimbHealthMap.Add("RHand", FLimbData(5.f));

    // Legs: mid-range, can bleed out but not instant kill
    LimbHealthMap.Add("LThigh", FLimbData(25.f));
    LimbHealthMap.Add("LCalf", FLimbData(20.f));
    LimbHealthMap.Add("LFoot", FLimbData(10.f));
    LimbHealthMap.Add("RThigh", FLimbData(25.f));
    LimbHealthMap.Add("RCalf", FLimbData(20.f));
    LimbHealthMap.Add("RFoot", FLimbData(10.f));

    // Torso or spine can be main kill zone if you want
    LimbHealthMap.Add("Torso", FLimbData(60.f));
}

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


//maybe make each limb have an amount of health?
//with amount of blood in it?

// - I like idea. Have been thinking about a bleed out mechanic where the zombie will bleed out and die after a realistic amount of time
// - so if u take out a leg and hit the femoral artery it will spurt blood out faster and the zombie will bleed out quicker than an arm being removed

void AZombieAi::ApplyLimbDamage(UPrimitiveComponent* HitComp, float Damage)
{
    FName LimbName = HitComp->GetFName();
    if (LimbHealthMap.Contains(LimbName))
    {
        FLimbData& Limb = LimbHealthMap[LimbName];
        Limb.CurrentHealth -= Damage;

        UE_LOG(LogTemp, Warning, TEXT("%s took %.2f damage (%.2f / %.2f)"),
            *LimbName.ToString(), Damage, Limb.CurrentHealth, Limb.MaxHealth);

        if (Limb.CurrentHealth <= Damage)
        {
            DismemberLimb(HitComp);

            //uncomment this once we've decided the XP shite
           /* AZombieGameCharacter* PlayerChar = Cast<AZombieGameCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
            PlayerChar->AddXP(5);*/

        }

        //add logic where if you lose upper arm you lose the rest

    }
}

void AZombieAi::DismemberLimb(UPrimitiveComponent* HitComp)
{
    FName LimbName = HitComp->GetFName();

    // Try to cast directly
    UStaticMeshComponent* LimbComp = Cast<UStaticMeshComponent>(HitComp);

    if (!LimbComp)
    {
        // Fallback: search through all attached static mesh components
        TArray<UActorComponent*> Components;
        GetComponents(UStaticMeshComponent::StaticClass(), Components);

        for (UActorComponent* C : Components)
        {
            if (C && C->GetFName() == LimbName)
            {
                LimbComp = Cast<UStaticMeshComponent>(C);
                break;
            }
        }
    }

    if (LimbComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Detaching limb: %s"), *LimbName.ToString());

        // Detach and enable physics
        LimbComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        LimbComp->SetSimulatePhysics(true);
        LimbComp->AddImpulse(FVector(0.f, 0.f, 200.f), NAME_None, true);


        //could be cool for the blood fx at first it spurts out loads then over time goes down as less blood in body

       /* if (BloodFX)
        {
            FVector BloodLoc = LimbComp->GetComponentLocation();
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodFX, BloodLoc, FRotator::ZeroRotator, FVector(1.f), true);
        }*/

       // LimbComp->SetLifeSpan(10.0f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DismemberLimb: Could not find component named %s"), *LimbName.ToString());
    } 
}

UBehaviorTree* AZombieAi::GetBehaviourTree() const
{
    return Tree;
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
            if (DistanceToPlayer > 1500.0f) // add variable for the range 
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