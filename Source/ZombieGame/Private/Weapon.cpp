// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::Shoot()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PlayerController || !Mesh) return;

    // Get direction from player's view
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector ShootDirection = CameraRotation.Vector();

    // Start from the muzzle socket
    FVector Start = Mesh->GetSocketLocation(TEXT("MuzzleFlashSocket"));
    FVector End = Start + (ShootDirection * 10000.0f); // 10,000 units forward from socket

    // Raycast
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    // Debug line
    DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);

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

