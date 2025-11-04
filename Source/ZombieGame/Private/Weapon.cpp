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
    float passThroughFactor = 0.2f;
    float damageScale = 0.01f;

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

    bulletVelocity = FMath::Sqrt(2.0f * muzzleEnergy / bulletMassKG) * 100.0f; // convert m/s to cm/s



    float KE_J = 0.5f * bulletMassKG * bulletVelocity;

    penetrationPower = C_pen * KE_J * sectionalDensity * constructionFactor;
    float depositedIfStopped_J = KE_J * stopFactor;
    fleshDamage = depositedIfStopped_J * damageScale;


    //this jsut for reading in m/s not cm/s
    float bulletVelocityMS = bulletVelocity / 100;

    UE_LOG(LogTemp, Warning,
        TEXT("Ballistics: E=%.1fJ, vel=%.2f m/s, Penetration=%.2f mm, FleshDamage(if stopped)=%.2f HP"), KE_J, bulletVelocityMS, penetrationPower, fleshDamage);
}

void AWeapon::Shoot()
{
    //Now shoots at socket however the bullet isnt centred

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

