// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Ammunition.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"


//need to fuck about with accuracy cose barrel length plays with it
//affects initial velocity and how far it spreads

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(mesh);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    // Get default properties from the ammo BP/class
    const AAmmunition* defaultAmmo = ammunitionType->GetDefaultObject<AAmmunition>();

    float powder = defaultAmmo->powderAmount;
    float grain = defaultAmmo->bulletGrain;
    float diameter = defaultAmmo->bulletDiameterMM;

    CalculateBallistics(powder, grain, diameter);

    int MagCount = 4;

    mags.Empty();
    for (int i = 0; i < MagCount; ++i)
    {
        mags.Add(magCapacity); // fill each mag to capacity
    }

    currentMagIndex = 0;

}

void AWeapon::CalculateBallistics(float powderAmount, float bulletGrain, float bulletDiameterMM)
{
    //later on implement different types of bullets, will affect pen
    //somehow implement how hollowpoints will work


    bulletVelocity = 0.0;
    float grainInKG = 0.00006479891;
    float energyDensity = 4.05e6;
    float effiency = 0.35;

    // Convert mass
    bulletMassKG = bulletGrain * grainInKG;
    float powderMassKG = powderAmount * grainInKG;
    float diameter_m = bulletDiameterMM * 0.001f;

    // Chemical energy in powder
    float chemicalEnergy = powderMassKG * energyDensity;

    // Muzzle energy (delivered)
    float muzzleEnergy = effiency * chemicalEnergy;

    float area = PI * FMath::Square(diameter_m * 0.5);

    // Sectional density (kg/m^2)
    float sectionalDensity = bulletMassKG / FMath::Max(area, 1e-12);

    velocityms = FMath::Sqrt(2.0f * muzzleEnergy / bulletMassKG); //m/s used for energy calculations
    float KE_J = 0.5f * bulletMassKG * FMath::Square(velocityms);

    bulletVelocity = velocityms * 100.0f;// for gameplay

    energyJoules = KE_J;

    //UE_LOG(LogTemp, Warning, TEXT("Ballistics: E=%.1fJ, vel=%.2f m/s (%.0f cm/s)"), KE_J, velocityms, bulletVelocity);

   //RECOIL
    float bulletMomentum = (bulletMassKG * velocityms) / 100;

    // Gas momentum
    float gasMassKG = powderMassKG * GasMassFraction;
    float gasVelocity = velocityms * GasVelocityMultiplier;
    float gasMomentum = gasMassKG * gasVelocity;

    // Total recoil impulse (N·s)
    recoilImpulse = bulletMomentum + gasMomentum;

    // Rearward velocity of weapon
    float recoilVelocity = recoilImpulse / WeaponMassKG; // m/s

    // Convert linear recoil to angular impulse
    float angularRecoil = recoilVelocity * StockLeverage;

    // Convert to gameplay camera degrees
    recoilKick = angularRecoil * RecoilToDegrees;
    recoilShake = angularRecoil * RecoilToDegrees * 0.55f;


}

void AWeapon::Shoot()
{
    //could either do animation or just use a slide to save time?
    //would mean only need reload animation and arm animation for shooting



    if (mags.Num() == 0 || currentMagIndex >= mags.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("No mags available!"));
        return;
    }

    if (mags[currentMagIndex] <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Current mag is empty! Reload needed."));
        return;
    }


    if (!ammunitionType) return;
    if (!mesh) return;

    const FName MuzzleSocketName = TEXT("Muzzle"); 
    const FVector MuzzleLocation = mesh->GetSocketLocation(MuzzleSocketName); 
    const FRotator MuzzleRotation = mesh->GetSocketRotation(MuzzleSocketName);
    FVector LaunchDirection = MuzzleRotation.Vector();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    
    AAmmunition* Bullet = GetWorld()->SpawnActor<AAmmunition>(ammunitionType, MuzzleLocation, MuzzleRotation, SpawnParams);
    if (Bullet)
    {
        Bullet->velocity = LaunchDirection * bulletVelocity;
        Bullet->energyJoules = energyJoules;
        Bullet->bulletMassKG = bulletMassKG;
        Bullet->velocityms = velocityms;
    }

    bool bIsLastRound = (mags[currentMagIndex] == 1);

    if (bIsLastRound && FireLastAnimation)
    {
        mesh->PlayAnimation(FireLastAnimation, false);
        bBoltLockedOpen = true;   // bolt now stays open
    }
    else if (FireAnimation)
    {
        mesh->PlayAnimation(FireAnimation, false);
    }


    mags[currentMagIndex]--;
    UE_LOG(LogTemp, Warning, TEXT("Fired! %d bullets left in current mag"), mags[currentMagIndex]);

    recoilPitch += recoilKick;
    recoilYaw += FMath::RandRange(-recoilShake, recoilShake);

   
   


    if (MuzzleFlash && mesh)
    {
        UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlash,
            mesh,
            TEXT("Muzzle"),                 
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            FVector(0.02, 0.02, 0.02),
            EAttachLocation::SnapToTarget,
            true
        );
    }


    //// Play sound if exists
    //if (FireSound)
    //{
    //	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    //}
}

void AWeapon::Reload()
{
    bool bWasEmpty = (mags[currentMagIndex] == 0);

    currentMagIndex++;

    if (mesh)
    {
        if (bWasEmpty && ReloadEmptyAnimation)
        {
            mesh->PlayAnimation(ReloadEmptyAnimation, false);
            bBoltLockedOpen = false;
        }
        else if (ReloadAnimation)
        {
            mesh->PlayAnimation(ReloadAnimation, false);
        }
    }
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

