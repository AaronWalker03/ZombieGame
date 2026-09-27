// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "CombatInterface.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::PerformMeleeTrace(float Damage, float Range, float Radius)
{
	FVector Start =
		GetActorLocation() +
		(GetActorForwardVector() * 50.0f);

	FVector End =
		Start +
		(GetActorForwardVector() * Range);

	TArray<FHitResult> HitResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), Start, Radius, 16, FColor::Green, false, 2.0f);
	DrawDebugSphere(GetWorld(), End, Radius, 16, FColor::Red, false, 2.0f);
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 2.0f);

	/*FString msg2 = FString::Printf(TEXT("DRAW DEBUG SPHERE"));
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, msg2);*/
#endif

	if (!bHit)
	{
		return;
	}

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();

		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActor->Implements<UCombatInterface>())
		{
			ICombatInterface::Execute_ReceiveDamage(
				HitActor,
				Damage);
		}
	}
}