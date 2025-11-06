// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Ammunition.h"
#include "Engine/World.h"


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

}

void AWeapon::CalculateBallistics(float powderAmount, float bulletGrain, float bulletDiameterMM)
{
    bulletVelocity = 0.0;
    float grainInKG = 0.00006479891;
    float energyDensity = 4.05e6;
    float effiency = 0.35;

    float C_pen = 0.00108f;
    float constructionFactor = 1.0f;

    float stopFactor = 0.9f;
    float damageScale = 0.02f;

    // Gelatin calibration: 7.62×39 FMJ (123 gr, ~760 m/s) penetrates ~30 cm of 10% gel
    const float gelReferenceDepth_m = 0.30f;  // 30 cm gel penetration at reference KE
    const float gelBlockThickness_m = 0.30f;  // Target body block thickness (30 cm)

    // Convert mass
    float bulletMassKG = bulletGrain * grainInKG;
    float powderMassKG = powderAmount * grainInKG;
    float diameter_m = bulletDiameterMM * 0.001f;

    // Chemical energy in powder
    float chemicalEnergy = powderMassKG * energyDensity;

    // Muzzle energy (delivered)
    float muzzleEnergy = effiency * chemicalEnergy;

    float area = PI * FMath::Square(diameter_m * 0.5);

    // Sectional density (kg/m^2)
    float sectionalDensity = bulletMassKG / FMath::Max(area, 1e-12);

    float velocity_m_s = 0.0f;

    velocity_m_s = FMath::Sqrt(2.0f * muzzleEnergy / bulletMassKG); //m/s used for energy calculations

    bulletVelocity = velocity_m_s * 100.0f;// for gameplay

    float KE_J = 0.5f * bulletMassKG * FMath::Square(velocity_m_s);

    float refVelocity_m_s = 760.0f;
    float refKE_J = 0.5f * bulletMassKG * FMath::Square(refVelocity_m_s);
    float gelEnergyPerMeter_Jpm = FMath::Max(refKE_J / gelReferenceDepth_m, 1.0f);

    float penetration_m = KE_J / gelEnergyPerMeter_Jpm;
    float penetration_cm = penetration_m * 100.0f; // Unreal world units

    // --- deposited energy ---
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

    penetrationPower = penetration_cm;
    fleshDamage = depositedEnergy_J * damageScale;

    UE_LOG(LogTemp, Warning,
        TEXT("Ballistics: E=%.1fJ, vel=%.2f m/s (%.0f cm/s), Penetration=%.1f cm, DepositedE=%.1fJ, FleshDamage=%.2f HP"),
        KE_J, velocity_m_s, bulletVelocity, penetration_cm, depositedEnergy_J, fleshDamage);
}

void AWeapon::Shoot()
{
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
        Bullet->fleshDamage = fleshDamage;
        Bullet->penetrationPower = penetrationPower;
    }


    //if (MuzzleFlash)
    //{
    //	UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, TEXT("MuzzleFlashSocket"));
    //}

    //// Play sound if exists
    //if (FireSound)
    //{
    //	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    //}
}

void AWeapon::Aim()
{
    //aim
}

void AWeapon::Reload()
{
    UE_LOG(LogTemp, Warning, TEXT("Base Weapon Reload"));
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

