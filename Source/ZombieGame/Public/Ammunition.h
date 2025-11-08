// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ammunition.generated.h"

UCLASS()
class ZOMBIEGAME_API AAmmunition : public AActor
{
	GENERATED_BODY()
	
public:	
	AAmmunition();

	//ideally when the player modifies a bullet from the specific calibre it creates a clone so they still have the default ammo
	//also implement fmj, armour pen and hollopoint bullets
	//^^^ this ontop of the option of subsonic or regular ammo
	//subsonic means quieter but not as fast as regular ammo
	//regular ammo is faster but also louder, when using a supressor it will still be loud asf in terms of hearing the bullet wizz past
	//basically if far away the zombies may not know your exact location but will anger them

protected:
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoMesh")
	UStaticMeshComponent* bulletTip;*/

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoMesh")
	UStaticMeshComponent* casing;*/

	//for velocity should probably implement barrel length as it does have a massive affect
	UPROPERTY(EditDefaultsOnly, Category = "Ammo Calculation Results")
	FVector velocity;
	UPROPERTY(EditDefaultsOnly, Category = "Ammo Calculation Results")
	float energyJoules;

	float fleshDamage;

	UPROPERTY(EditDefaultsOnly, Category = "AmmoProperties")
	float powderAmount;

	float velocityms;
	float lifetime = 5.0f; // seconds before despawn
	float gravity = -980.f;

	FVector previousPosition;

	float bulletMassKG;

	UPROPERTY(EditDefaultsOnly, Category = "AmmoProperties")
	float bulletGrain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AmmoProperties")
	float bulletDiameterMM;

	bool PenetrationTest(float KE_J, AActor* HitActor, UPrimitiveComponent* HitComponent, const FVector& HitLocation);
};
