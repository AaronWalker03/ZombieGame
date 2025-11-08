// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammunition.h"
#include <Kismet/GameplayStatics.h>
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"

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

bool AAmmunition::PenetrationTest(float KE_J, AActor* HitActor, UPrimitiveComponent* HitComponent, const FVector& HitLocation)
{
    //potentially optimise this by running it on the gpu

    // maybe add bone damage, if only hits side of the limb shouldnt be able to dismember
    //bone breaking will allow for more realistic dismemberemnt


    float damageScale = 0.02f;
    bool canGoThrough = false;

    FString ActorName = HitActor->GetName();

    USceneComponent* HitLimb = Cast<USceneComponent>(HitComponent);
    float limbThickness_m = 0.0f;

    FVector ComponentExtent = HitLimb->Bounds.BoxExtent; // half-size in cm
    limbThickness_m = ComponentExtent.Size() * 0.01f;

    float refKE_J = 0.5f * bulletMassKG * FMath::Square(velocityms);

    float fleshEnergyPerMeter_Jpm = FMath::Max(refKE_J / 0.3f, 1.0f);

    float penetration_m = KE_J / fleshEnergyPerMeter_Jpm;

   // float penetration_cm = penetration_m * 100.0f; // Unreal world units

    float depositedEnergy_J = 0.0f;

    if (penetration_m >= limbThickness_m)
    {     
        depositedEnergy_J = fleshEnergyPerMeter_Jpm * limbThickness_m;

        float remainingKE_J = KE_J - depositedEnergy_J;

        velocityms = FMath::Sqrt((2.0f * remainingKE_J) / bulletMassKG);

        canGoThrough = true;
    }
    else
    {
        // Bullet stops inside the block: all kinetic energy is deposited
        depositedEnergy_J = KE_J;

        canGoThrough = false;
    }

    fleshDamage = depositedEnergy_J * damageScale;

    UE_LOG(LogTemp, Warning, TEXT("[PenetrationTest] Hit Actor: %s | Limb Thickness: %.2f m | Damage: %.2f | CanGoThrough: %s"),
        *ActorName, limbThickness_m, fleshDamage, canGoThrough ? TEXT("Yes") : TEXT("No"));

    return canGoThrough;

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
        if (Hit.GetActor())
        {
            // Debug: speed before
            float speedBefore = velocityms;

            bool canGoThrough = PenetrationTest(energyJoules, Hit.GetActor(), Hit.GetComponent(), Hit.ImpactPoint);

            // Debug: speed after
            float speedAfter = velocityms;
            UE_LOG(LogTemp, Warning, TEXT("[BulletHit] Speed before: %.2f m/s | Speed after: %.2f m/s"), speedBefore, speedAfter);

            if (canGoThrough)
            {
                //slight bug that isnt allowing collateral penetration
            }
            else
            {
                // Bullet stops inside the limb
                Destroy();
                return;
            }
        }     
    }

    velocity.Z += gravity * DeltaTime; 
    SetActorLocation(End);
    previousPosition = End;

    //assuming over time bullet loses velocity over time because of air friction


	//could do sound of each bullet wizzing past in here?
	//wizzing bullet sound with attenuation and doppler effect
	//Stop playing sound once bullet impacts surface or gets too far away
}