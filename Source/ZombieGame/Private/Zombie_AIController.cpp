// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie_AIController.h"
#include "ZombieAi.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ZombieGameCharacter.h"

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
		}
	}
}

void AZombie_AIController::SetupPerceptionSystem()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

	if (SightConfig)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));

		SightConfig->SightRadius = 500.0f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 25.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AZombie_AIController::OnTargetDetected);
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}
}

void AZombie_AIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	if (AZombieGameCharacter* const Player = Cast<AZombieGameCharacter>(Actor))
	{
		GetBlackboardComponent()->SetValueAsBool("PlayerVisible", Stimulus.WasSuccessfullySensed());
	}
}
