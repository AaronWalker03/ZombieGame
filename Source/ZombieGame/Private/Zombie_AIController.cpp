// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie_AIController.h"
#include "ZombieAi.h"

AZombie_AIController::AZombie_AIController(FObjectInitializer const& ObjectInitializer)
{
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
