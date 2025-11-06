// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammunition.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AAmmunition::AAmmunition()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	//bulletTip = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletTip"));
	//bulletTip->SetupAttachment(RootComponent);

	// Casing mesh
	/*casing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Casing"));
	casing->SetupAttachment(RootComponent);*/
}

// Called when the game starts or when spawned
void AAmmunition::BeginPlay()
{
	Super::BeginPlay();
	previousPosition = GetActorLocation();
}

// Called every frame
void AAmmunition::Tick(float DeltaTime)
{
    lifetime -= DeltaTime;
    if (lifetime <= 0.f)
    {
        Destroy();
        return;
    }

    // Ensure PreviousPosition initialized (in case BeginPlay wasn't used)
    if (previousPosition.IsNearlyZero())
    {
        previousPosition = GetActorLocation();
    }

    // Compute new position based on current velocity
    FVector Start = previousPosition;
    FVector End = Start + velocity * DeltaTime;

    // Trace params - ignore self and the weapon owner
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (GetOwner()) Params.AddIgnoredActor(GetOwner());
    Params.bReturnPhysicalMaterial = false;

    // Perform line trace between previous and new position
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    // Debug: draw path
    DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.5f);

    if (bHit)
    {
        // Immediate, simple hit handling — apply point damage if actor exists
        if (Hit.GetActor())
        {
            UGameplayStatics::ApplyPointDamage(
                Hit.GetActor(),
                fleshDamage,                       // damage amount
                velocity.GetSafeNormal(),          // shot direction
                Hit,
                GetInstigatorController(),
                this,
                nullptr                            // optional damage type class
            );
        }

        // Place an impact debug point
        DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.0f, FColor::Red, false, 2.0f);

        // Destroy bullet immediately on first hit
        Destroy();
        return;
    }

    // No hit: apply gravity and move to end position
    velocity.Z += gravity * DeltaTime; // Gravity should be negative, e.g. -980.f (cm/s^2)
    SetActorLocation(End);
    previousPosition = End;

    //assuming over time bullet loses velocity over time because of air friction


	//could do sound of each bullet wizzing past in here?
	//Play whistling bullet sound with attenuation and doppler effect
	//Stop playing sound once bullet impacts surface or gets too far away
}