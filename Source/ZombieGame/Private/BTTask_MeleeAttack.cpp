// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MeleeAttack.h"
#include "Zombie_AIController.h"
#include "ZombieAi.h"

EBTNodeResult::Type UBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AZombie_AIController* AIController = Cast<AZombie_AIController>(
        OwnerComp.GetAIOwner());

    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    AZombieAi* Zombie = Cast<AZombieAi>(AIController->GetPawn());

    if (!Zombie)
    {
        return EBTNodeResult::Failed;
    }

    Zombie->MeleeAttack_Implementation();

    return EBTNodeResult::Succeeded;
}
