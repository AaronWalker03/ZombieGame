// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Zombie_AIController.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAME_API AZombie_AIController : public AAIController
{
	GENERATED_BODY()
	
public:
	explicit AZombie_AIController(FObjectInitializer const& ObjectInitializer);

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	class UAISenseConfig_Sight* SightConfig;
	class UAISenseConfig_Hearing* HearingConfig;

	void SetupPerceptionSystem();

	UFUNCTION()
	void OnTargetSpotted(AActor* Actor, FAIStimulus const Stimulus);

	UFUNCTION()
	void OnTargetHeard(AActor* Actor, FAIStimulus const Stimulus);
};
