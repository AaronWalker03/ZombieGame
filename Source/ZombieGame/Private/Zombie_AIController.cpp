// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie_AIController.h"
#include "ZombieAi.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ZombieGameCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

AZombie_AIController::AZombie_AIController(FObjectInitializer const& ObjectInitializer)
{
	SetupPerceptionSystem();
}

void AZombie_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AZombieAi* const zombie = Cast<AZombieAi>(InPawn))
	{
		if (UBehaviorTree* const tree = zombie->GetBehaviourTree())
		{
			UBlackboardComponent* b;
			UseBlackboard(tree->BlackboardAsset, b);
			Blackboard = b;
			RunBehaviorTree(tree);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("POSSESS CALLED"));
			}

			GetBlackboardComponent()->SetValueAsVector(
				TEXT("TargetLocation"),
				GetPawn()->GetActorLocation() - FVector(400, 0, 0)
			);

			GetWorld()->GetTimerManager().SetTimer(
				TargetRetryHandle,
				this,
				&AZombie_AIController::TrySetInitialTarget,
				0.5f,   // check every half second
				true
			);
		}
	}
}

void AZombie_AIController::BeginPlay()
{
	Super::BeginPlay();

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (!NavSys)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("NO NAV SYSTEM"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("NAV SYSTEM OK"));
	}

	/*UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
		GetWorld(),
		GetPawn()->GetActorLocation(),
		GetPawn()->GetActorLocation() + FVector(300, 0, 0)
	);*/

	/*if (!Path || Path->PathPoints.Num() == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("NO VALID PATH"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PATH VALID"));
	}*/
	//MoveToLocation(GetPawn()->GetActorLocation() + FVector(300, 0, 0));
}

void AZombie_AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTestedMove)
	{
		bTestedMove = true;
		EPathFollowingRequestResult::Type Result = MoveToLocation(GetPawn()->GetActorLocation() + FVector(800, 0, 0));

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Yellow,
			FString::Printf(TEXT("Move result: %d"), (int)Result)
		);
	}

	/*if (GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("MOVE IDLE"));
	}

	if (!GetPawn())
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("NO PAWN"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("HAS PAWN"));
	}

	if (GetPawn())
	{
		UNavMovementComponent* NavMove = GetPawn()->FindComponentByClass<UNavMovementComponent>();

		if (!NavMove)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("NO NAV MOVEMENT COMPONENT"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("HAS NAV MOVEMENT"));
		}
	}*/
}

void AZombie_AIController::TrySetInitialTarget()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	if (PlayerPawn && GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("TargetPlayer"), PlayerPawn);

		// Stop trying once found
		GetWorld()->GetTimerManager().ClearTimer(TargetRetryHandle);
	}
}

void AZombie_AIController::SetupPerceptionSystem()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SetPerceptionComponent(*PerceptionComponent);

	if (SightConfig)
	{
		SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
		SightConfig->SightRadius = 500.0f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 25.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.0f); // Length of time till stimulus is forgotten
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	}

	if (HearingConfig)
	{
		HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
		HearingConfig->HearingRange = 700.0f;
		HearingConfig->SetMaxAge(15.0f);

		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	}

	// Register senses
	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->ConfigureSense(*HearingConfig);

	PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());

	PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AZombie_AIController::OnPerceptionUpdated);

	/*if (SightConfig && HearingConfig)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AZombie_AIController::OnTargetSpotted);
		GetPerceptionComponent()->ConfigureSense(*SightConfig);

		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AZombie_AIController::OnTargetHeard);
		GetPerceptionComponent()->ConfigureSense(*HearingConfig);
	}*/
}

void AZombie_AIController::OnPerceptionUpdated(const TArray<AActor*>& updatedActors)
{
	bool bCanSeePlayer = false;
	AZombieGameCharacter* SeenPlayer = nullptr;

	for (AActor* Actor : updatedActors)
	{
		if (AZombieGameCharacter* Player = Cast<AZombieGameCharacter>(Actor))
		{
			FActorPerceptionBlueprintInfo Info;
			GetPerceptionComponent()->GetActorsPerception(Player, Info);

			for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
			{
				if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
				{
					bCanSeePlayer = Stimulus.WasSuccessfullySensed();
					SeenPlayer = Player;
				}
			}
		}
	}

	// ONLY write if value actually changed
	if (GetBlackboardComponent()->GetValueAsBool("PlayerVisible") != bCanSeePlayer)
	{
		GetBlackboardComponent()->SetValueAsBool("PlayerVisible", bCanSeePlayer);
	}

	if (bCanSeePlayer)
	{
		GetBlackboardComponent()->SetValueAsObject("TargetPlayer", SeenPlayer);
	}
}

void AZombie_AIController::OnTargetSpotted(AActor* Actor, FAIStimulus const Stimulus)
{
	if (AZombieGameCharacter* const Player = Cast<AZombieGameCharacter>(Actor))
	{
		GetBlackboardComponent()->SetValueAsBool("PlayerVisible", Stimulus.WasSuccessfullySensed());
		GetBlackboardComponent()->SetValueAsObject("TargetPlayer", Player);
	}
}

void AZombie_AIController::OnTargetHeard(AActor* Actor, FAIStimulus const Stimulus)
{
	if (AZombieGameCharacter* const Player = Cast<AZombieGameCharacter>(Actor))
	{
		GetBlackboardComponent()->SetValueAsBool("PlayerHeard", Stimulus.WasSuccessfullySensed());
	}
}
