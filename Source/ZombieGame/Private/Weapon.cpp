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
    float grainInKG = 0.00006479891;
    float energyDensity = 4.05e6;
    float effiency = 0.30;
    float kP = 1e-6;
    float kD = 0.01;
    float tissueEffiency = 1.0;     // soft tissue multiplier
    float expansion = 1.0; // 1.0 for FMJ, >1 for hollowpoint

    // Convert mass
    float bulletMass = bulletGrain * grainInKG;
    float powderMass = powderAmount * grainInKG;

    // Chemical energy in powder
    float chemicalEnergy = powderMass * energyDensity;

    // Muzzle energy (delivered)
    float muzzleEnergy = effiency * chemicalEnergy;

    // Frontal area (m^2)
    float d_m = bulletDiameterMM * 0.001;
    float area = PI * FMath::Square(d_m * 0.5);

    // Sectional density (kg/m^2)
    float sD = bulletMass / FMath::Max(area, 1e-12);

    // Penetration power 
    float pen = kP * (muzzleEnergy / FMath::Max(area, 1e-12)) * sD;

    // Flesh damage
    float flesh = kD * muzzleEnergy * expansion * tissueEffiency;

    // Velocity (m/s)
    bulletVelocity = 0.0;
    if (bulletMass > 0.0)
    {
        bulletVelocity = FMath::Sqrt(2.0 * muzzleEnergy / bulletMass);
    }

    //final product
    penetrationPower = pen;
    fleshDamage = flesh;

    UE_LOG(LogTemp, Warning, TEXT("Calculated Ballistics: Penetration = %f, FleshDamage = %f, bulletVelocity = %f"),
        penetrationPower,
        fleshDamage,
        bulletVelocity);
}

void AWeapon::Shoot()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PlayerController || !mesh) return;

    // Get direction from player's view
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector ShootDirection = CameraRotation.Vector();

    // NOT ACTUALLY SHOOTING FROM NOZZLE FOR SOME REASON
    FVector MuzzleLocation = mesh->GetSocketLocation(TEXT("Muzzle"));
    FRotator MuzzleRotation = ShootDirection.Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    
    AAmmunition* SpawnedBullet = GetWorld()->SpawnActor<AAmmunition>(ammunitionType, MuzzleLocation, MuzzleRotation, SpawnParams);
    if (SpawnedBullet)
    {
        SpawnedBullet->totalBulletVelocity = bulletVelocity;
        SpawnedBullet->totalFleshDamage = fleshDamage;
        SpawnedBullet->totalPenetrationPower = penetrationPower;

        if (SpawnedBullet->bulletTip)
        {
            SpawnedBullet->bulletTip->SetSimulatePhysics(true);
            SpawnedBullet->bulletTip->SetPhysicsLinearVelocity(ShootDirection * bulletVelocity);
        }
    }

    //lets try projectile based first see what performance is like
    // //if we do projectile we dont need to worry about calculating bullet drop physics, will do that for us
    // if projectiles are too computationally expensive then we need to do with line trace which is less expensive but need to do vector calculations for drop 
    // will also need to calculate how fast the line trace can do shit because of velocity
    //// Raycast
    //FHitResult Hit;
    //FCollisionQueryParams Params;
    //Params.AddIgnoredActor(this);
    //Params.AddIgnoredActor(GetOwner());

    //bool bHit = GetWorld()->LineTraceSingleByChannel(
    //    Hit,
    //    Start,
    //    End,
    //    ECC_Visibility,
    //    Params
   // );

    // Debug line
   // DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);

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

