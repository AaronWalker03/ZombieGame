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
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "ZombieSpawner.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AZombieAi::AZombieAi()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    pawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

    MaxHealth = 100.0f;

    CurrentHealth = MaxHealth;

    bloodQuantity = 5.0f; // 5 Litres is the average amount in a human

    bleedOutRate = 2.0f;

    damage = 10.f;

    CurrentState = EZombieState::ZS_Idle;

    SetBodyparts();
}

void AZombieAi::SetBodyparts()
{
    TorsoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TorsoMesh"));
    TorsoMesh->SetupAttachment(GetMesh(), FName("Torso"));

    HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
    HeadMesh->SetupAttachment(TorsoMesh); // attach to head socket

    //Setup Left Arm Attachments
    LUpArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LUpArm"));
    LUpArm->SetupAttachment(TorsoMesh);

    LForearm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LForearm"));
    LForearm->SetupAttachment(LUpArm);

    LHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LHand"));
    LHand->SetupAttachment(LForearm);

    //Setup Right Arm Attachments
    RUpArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RUpArm"));
    RUpArm->SetupAttachment(TorsoMesh);

    RForearm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RForearm"));
    RForearm->SetupAttachment(RUpArm);

    RHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RHand"));
    RHand->SetupAttachment(RForearm);

    //Setup Left Leg Attachments
    LThigh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LThigh"));
    LThigh->SetupAttachment(TorsoMesh);

    LCalf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LCalf"));
    LCalf->SetupAttachment(LThigh);

    LFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LFoot"));
    LFoot->SetupAttachment(LCalf);

    //Setup Right Leg Attachments
    RThigh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RThigh"));
    RThigh->SetupAttachment(TorsoMesh);

    RCalf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RCalf"));
    RCalf->SetupAttachment(RThigh);

    RFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RFoot"));
    RFoot->SetupAttachment(RCalf);

    GetMesh()->SetVisibility(false);

    LimbHealthMap.Add("HeadMesh", FLimbData(99.f, 1.5f));

    // Arms: low importance
    LimbHealthMap.Add("LUpArm", FLimbData(15.f, 0.5f)); // First Number Health, Second how fast the zombie will bleed out from losing that appendage
    LimbHealthMap.Add("LForearm", FLimbData(10.f, 0.1f));
    LimbHealthMap.Add("LHand", FLimbData(5.f, 0.01f));
    LimbHealthMap.Add("RUpArm", FLimbData(15.f, 0.5f));
    LimbHealthMap.Add("RForearm", FLimbData(10.f, 0.1f));
    LimbHealthMap.Add("RHand", FLimbData(5.f, 0.01f));

    // Legs: mid-range, can bleed out but not instant kill
    LimbHealthMap.Add("LThigh", FLimbData(25.f, 1.0f));
    LimbHealthMap.Add("LCalf", FLimbData(20.f, 0.2f));
    LimbHealthMap.Add("LFoot", FLimbData(10.f, 0.1f));
    LimbHealthMap.Add("RThigh", FLimbData(25.f, 1.0f));
    LimbHealthMap.Add("RCalf", FLimbData(20.f, 0.2f));
    LimbHealthMap.Add("RFoot", FLimbData(10.f, 0.1f));

    // Torso or spine can be main kill zone if you want
    LimbHealthMap.Add("TorsoMesh", FLimbData(60.f, 0.3f));
}

void AZombieAi::BeginPlay()
{
    Super::BeginPlay();

    /*pawnSensingComp->SensingInterval = 0.5f;
    pawnSensingComp->bSeePawns = true;*/

    // Default stats
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
        //pawnSensingComp->OnSeePawn.AddDynamic(this, &AZombieAi::HandleSeePlayer);
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

        FString msg = FString::Printf(TEXT("%s took %.2f damage (%.2f / %.2f)"),
            *LimbName.ToString(), Damage, Limb.CurrentHealth, Limb.MaxHealth);
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, msg);

        if (Limb.CurrentHealth <= Damage)
        {
            DismemberLimb(HitComp);

            FTimerDelegate BleedDelegate;
            BleedDelegate.BindUFunction(
                this,
                FName("ApplyBleed"),
                Limb.BleedSeverity
            );

            GetWorldTimerManager().SetTimer(
                TimerHandle,
                BleedDelegate,
                bleedOutRate,
                true
            );

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

        if (LimbName != "TorsoMesh")
        {
            // Detach and enable physics
            LimbComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            LimbComp->SetSimulatePhysics(true);
            LimbComp->AddImpulse(FVector(0.f, 0.f, 200.f), NAME_None, true);
        }

        //could be cool for the blood fx at first it spurts out loads then over time goes down as less blood in body

        if (BloodFX)
        {
            FVector BloodLoc = LimbComp->GetComponentLocation();
            UNiagaraComponent* NiagaraComp =
                UNiagaraFunctionLibrary::SpawnSystemAttached(
                    BloodFX,
                    LimbComp,
                    NAME_None,
                    FVector::ZeroVector,
                    FRotator::ZeroRotator,
                    EAttachLocation::KeepRelativeOffset,
                    true
                );

            //UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodFX, BloodLoc, FRotator::ZeroRotator, FVector(1.f), true);
        }

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

int AZombieAi::MeleeAttack_Implementation()
{
    return 0;
}

void AZombieAi::SetSpawner(AZombieSpawner* inSpawner)
{
    Spawner = inSpawner;
}

void AZombieAi::KillZombie()
{
    if (bIsDead) return;

    TArray<UActorComponent*> Components;
    GetComponents(UStaticMeshComponent::StaticClass(), Components);

    for (UActorComponent* C : Components)
    {
        if (C)
        {
            UStaticMeshComponent* LimbComp = Cast<UStaticMeshComponent>(C);

            if (!LimbComp->IsSimulatingPhysics())
            {
                LimbComp->SetSimulatePhysics(true);
            }
        }
    }

    if (Spawner) Spawner->NotifyZombieDied();

    bIsDead = true;

    AAIController* aiController = Cast<AAIController>(GetController());

    if (aiController)
    {
        aiController->StopMovement();
        aiController->GetBrainComponent()->StopLogic(TEXT("Zombie Dead"));
    }

    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // After next round, Destroy actor and components if we want to allow the bodies to pile up. Or just set a timer to Call Destroy actor

    GetWorldTimerManager().SetTimer(
       TimerHandle,
       this,
       &AZombieAi::DestroyZombie,
       5.0f,
       false
    );

    
}

void AZombieAi::DestroyZombie()
{
    Destroy();
}

void AZombieAi::ApplyBleed(float bleedSeverity)
{
    if (bloodQuantity >= 0)
    {
        bloodQuantity -= bleedSeverity;
    }
}

void AZombieAi::SetCurrentHealth(float health)
{
    CurrentHealth = health;
}

float AZombieAi::GetCurrentHealth()
{
    return CurrentHealth;
}

void AZombieAi::SetCrawlMode()
{
    // Rotate to the floor and face player for now before animations
    GetMesh()->SetRelativeRotation(FRotator(0, -90, 90));

    // Move down to the ground
    GetMesh()->SetRelativeLocation(FVector(0, 0, -40));

    // Slow them down
    GetCharacterMovement()->MaxWalkSpeed = movementSpeed / 2;
}

// Called every frame
void AZombieAi::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // lastKnownPlayerLocation = TargetPlayer->GetActorLocation

    //switch (CurrentState)
    //{
    //case EZombieState::ZS_Idle:
    //    // Maybe look around or play idle animation
    //    break;

    //case EZombieState::ZS_Wandering:
    //    // Move around randomly

    //    //Could make it so that it keeps going to the last known location of the player
    //    //roam around for a bit then to the original spawn location?

    //   // AiController->MoveTo = lastKnownPlayerLocation

    //    //once at location roam around like a fent addict
    //  

    //    break;

    //case EZombieState::ZS_Attack:
    //    if (TargetPlayer)
    //    {
    //        float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    //        // If too far, stop chasing
    //        if (DistanceToPlayer > 1500.0f) // add variable for the range 
    //        {
    //            TargetPlayer = nullptr;
    //            CurrentState = EZombieState::ZS_Wandering;

    //            AAIController* AIController = Cast<AAIController>(GetController());
    //            if (AIController)
    //            {
    //                AIController->StopMovement();
    //            }
    //        }
    //    }
    //    break;
    //}

    //if (pawnSensingComp)
    //{
    //    DrawDebugCone(
    //        GetWorld(),
    //        GetActorLocation(),
    //        GetActorForwardVector(),
    //        pawnSensingComp->SightRadius,
    //        FMath::DegreesToRadians(pawnSensingComp->GetPeripheralVisionAngle() / 2.0f), // half-angle
    //        FMath::DegreesToRadians(pawnSensingComp->GetPeripheralVisionAngle() / 2.0f), // half-angle
    //        12,
    //        FColor::Green,
    //        false,
    //        0.1f,
    //        0,
    //        1.0f
    //    );
    //}

    if (bloodQuantity <= 0.0f || CurrentHealth <= 0.0f)
    {
        KillZombie();
    }

    if (LFoot->IsSimulatingPhysics() && RFoot->IsSimulatingPhysics())
    {
        SetCrawlMode();
    }

    FString msg = FString::Printf(TEXT("Health: %f  Blood: %f"), CurrentHealth, bloodQuantity);
    GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, msg);

    // May need to add in a check here for if the zombie has both legs blown off for changing movement state to crawling or something along those lines
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