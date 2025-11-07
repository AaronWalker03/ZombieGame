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

void AAmmunition::PenetrationTest(float KE_J, AActor* HitActor, UPrimitiveComponent* HitComponent, const FVector& HitLocation)
{
    float damageScale = 0.02f;


    if (HitActor)
    {
        FString ActorName = HitActor->GetName();
        FString ComponentName = HitComponent ? HitComponent->GetName() : TEXT("None");

        // If this is a skeletal mesh, try to get the bone name
        FString BoneName = TEXT("N/A");
        if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(HitComponent))
        {
            // Get the closest bone to the impact location
            FName ClosestBone = SkelComp->FindClosestBone(HitLocation);
            if (ClosestBone != NAME_None)
            {
                BoneName = ClosestBone.ToString();
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("[PenetrationTest] Hit Actor: %s | Component: %s | Bone: %s | Location: %s"),
            *ActorName,
            *ComponentName,
            *BoneName,
            *HitLocation.ToString());
    }

    //get reference of body part?

    //initially done with gel block to imitate flesh
    float gelReferenceDepth_m = 0.30f;  
    float gelBlockThickness_m = 0.30f;  

    float refKE_J = 0.5f * bulletMassKG * FMath::Square(velocityms);
    float gelEnergyPerMeter_Jpm = FMath::Max(refKE_J / gelReferenceDepth_m, 1.0f);

    float penetration_m = KE_J / gelEnergyPerMeter_Jpm;
    float penetration_cm = penetration_m * 100.0f; // Unreal world units

    float depositedEnergy_J = 0.0f;

    if (penetration_m >= gelBlockThickness_m)
    {
        // Bullet exits the block: energy lost inside the block = J per meter * thickness (meters)
        depositedEnergy_J = gelEnergyPerMeter_Jpm * gelBlockThickness_m;
    }
    else
    {
        // Bullet stops inside the block: all kinetic energy is deposited
        depositedEnergy_J = KE_J;
    }


    fleshDamage = depositedEnergy_J * damageScale;



    //then do damage here?
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
           //if hit check penetration
            PenetrationTest(energyJoules, Hit.GetActor(), Hit.GetComponent(), Hit.ImpactPoint);                  
        }

        // Place an impact debug point
        DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.0f, FColor::Red, false, 2.0f);

        // Destroy bullet immediately on first hit
        //additionally check penetration, if can go through keep going dont destroy
        Destroy();
        return;
    }

    // No hit: apply gravity and move to end position
    velocity.Z += gravity * DeltaTime; // Gravity should be negative, e.g. -980.f (cm/s^2)
    SetActorLocation(End);
    previousPosition = End;

    //assuming over time bullet loses velocity over time because of air friction


	//could do sound of each bullet wizzing past in here?
	//wizzing bullet sound with attenuation and doppler effect
	//Stop playing sound once bullet impacts surface or gets too far away
}