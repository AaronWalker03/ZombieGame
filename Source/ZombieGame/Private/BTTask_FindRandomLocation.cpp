// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindRandomLocation.h"
#include "Zombie_AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindRandomLocation::UBTTask_FindRandomLocation(FObjectInitializer const& ObjectInitializer)
{
	NodeName = "Find Random Location";
}

EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// get AI controller and its zombie
	if (AZombie_AIController* const Controller = Cast<AZombie_AIController>(OwnerComp.GetAIOwner()))
	{
		if (APawn* const Zombie = Controller->GetPawn())
		{
			// obtain zombie location to use as an origin
			FVector const Origin = Zombie->GetActorLocation();

			// get the navigation system and generate a random location
			if (UNavigationSystemV1* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
			{
				FNavLocation Location;
				if (NavSys->GetRandomPointInNavigableRadius(Origin, SearchRadius, Location))
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Location.Location);

					// Finish this task successfully
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

					// Rest of sequence still in progress
					return EBTNodeResult::InProgress;
				}

				// finish with success

				/*if (Zombie->GetActorLocation() == Location)
				{
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
					return EBTNodeResult::Succeeded;
				}*/
			}
		}
	}
	
	return EBTNodeResult::Failed;
}
