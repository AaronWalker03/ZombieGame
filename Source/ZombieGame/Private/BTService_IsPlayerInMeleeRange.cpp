// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_IsPlayerInMeleeRange.h"
#include "Zombie_AIController.h"
#include "ZombieAi.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_IsPlayerInMeleeRange::UBTService_IsPlayerInMeleeRange()
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("Is Player In Melee Range");
}

void UBTService_IsPlayerInMeleeRange::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// get controller and AI
	const AZombie_AIController* Controller = Cast<AZombie_AIController>(OwnerComp.GetAIOwner());
	const AZombieAi* Zombie = Cast<AZombieAi>(Controller->GetPawn());

	// get player character
	const ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	// set blackboard key depending on whether or not the player is in melee range
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), Zombie->GetDistanceTo(Player) <= MeleeRange);
}
