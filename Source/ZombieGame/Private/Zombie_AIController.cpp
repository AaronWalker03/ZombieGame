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
			RunBehaviorTree(tree);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("POSSESS CALLED"));
			}

			UE_LOG(LogTemp, Warning, TEXT("%s sees player"), *GetName());

			//GetBlackboardComponent()->SetValueAsVector(
			//	TEXT("TargetLocation"),
			//	GetPawn()->GetActorLocation() - FVector(400, 0, 0)
			//);

			//GetWorld()->GetTimerManager().SetTimer(
			//	TargetRetryHandle,
			//	this,
			//	&AZombie_AIController::TrySetInitialTarget,
			//	0.5f,   // check every half second
			//	true
			//);
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

	AActor* Target = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));

	FString State = Target ? TEXT("CHASE") : TEXT("PATROL");

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.0f,
			Target ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("%s -> %s"), *GetName(), *State)
		);
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Check if we currently see the player
	TArray<AActor*> PerceivedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	bool bCurrentlySeeingPlayer = false;

	for (AActor* Actor : PerceivedActors)
	{
		if (Cast<AZombieGameCharacter>(Actor))
		{
			bCurrentlySeeingPlayer = true;
			break;
		}
	}

	// Only forget if NOT seeing AND timer expired
	if (!bCurrentlySeeingPlayer && (CurrentTime - LastSeenTime > SightConfig->GetMaxAge()))
	{
		GetBlackboardComponent()->ClearValue("TargetPlayer");
	}
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

		SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 50.0f;
		SightConfig->PeripheralVisionAngleDegrees = 70.0f;
		SightConfig->SetMaxAge(5.0f); // Length of time till stimulus is forgotten
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	

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
	//PerceptionComponent->ConfigureSense(*HearingConfig);

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
					bCanSeePlayer |= Stimulus.WasSuccessfullySensed();
					SeenPlayer = Player;
				}
			}
		}
	}

	if (SeenPlayer)
	{
		// LOCK TARGET
		GetBlackboardComponent()->SetValueAsObject("TargetPlayer", SeenPlayer);

		LastSeenTime = GetWorld()->GetTimeSeconds();
	}

	//// ONLY write if value actually changed
	//if (GetBlackboardComponent()->GetValueAsBool("PlayerVisible") != bCanSeePlayer)
	//{
	//	GetBlackboardComponent()->SetValueAsBool("PlayerVisible", bCanSeePlayer);
	//}

	//if (bCanSeePlayer)
	//{
	//	GetBlackboardComponent()->SetValueAsObject("TargetPlayer", SeenPlayer);
	//}
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
