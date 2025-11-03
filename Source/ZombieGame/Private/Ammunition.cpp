// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammunition.h"

// Sets default values
AAmmunition::AAmmunition()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	bulletTip = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletTip"));
	bulletTip->SetupAttachment(RootComponent);

	// Casing mesh
	/*casing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Casing"));
	casing->SetupAttachment(RootComponent);*/
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

    //assuming over time bullet loses velocity over time because of air friction


	//could do sound of each bullet wizzing past in here?
	//Play whistling bullet sound with attenuation and doppler effect
	//Stop playing sound once bullet impacts surface or gets too far away
}

