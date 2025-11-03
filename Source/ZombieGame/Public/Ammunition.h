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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoMesh")
	UStaticMeshComponent* bulletTip;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoMesh")
	UStaticMeshComponent* casing;*/

	//for velocity should probably implement barrel length as it does have a massive affect
	UPROPERTY(EditDefaultsOnly, Category = "Ammo Calculation Results")
	float totalBulletVelocity;
	UPROPERTY(EditDefaultsOnly, Category = "Ammo Calculation Results")
	float totalPenetrationPower;
	UPROPERTY(EditDefaultsOnly, Category = "Ammo Calculation Results")
	float totalFleshDamage;

	UPROPERTY(EditDefaultsOnly, Category = "AmmoProperties")
	float powderAmount;

	UPROPERTY(EditDefaultsOnly, Category = "AmmoProperties")
	float bulletGrain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AmmoProperties")
	float bulletDiameterMM;
};
