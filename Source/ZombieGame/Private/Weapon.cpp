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

    UE_LOG(LogTemp, Warning, TEXT("Ballistics: E=%.1fJ, vel=%.2f m/s (%.0f cm/s)"), KE_J, velocityms, bulletVelocity);
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
        Bullet->energyJoules = energyJoules;
        Bullet->bulletMassKG = bulletMassKG;
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

