// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammunition.h"

// Sets default values
AAmmunition::AAmmunition()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	fullAmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(fullAmmoMesh);
}

// Called when the game starts or when spawned
void AAmmunition::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAmmunition::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	//could do sound of each bullet wizzing past in here?
	//Play whistling bullet sound with attenuation and doppler effect
	//Stop playing sound once bullet impacts surface or gets too far away
}

