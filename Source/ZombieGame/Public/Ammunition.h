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

	//ideally turn this to a skeletal mesh so can in depth modify the bullet components

	UPROPERTY(VisibleAnywhere, Category = "AmmoMesh")
	UStaticMeshComponent* fullAmmoMesh;

	//to use for ejecting animations
	UPROPERTY(VisibleAnywhere, Category = "AmmoMesh") // ~~ idk if all these variable names that are similar will get very confusing and could u use niagra particles for ejection animations ~~
	UStaticMeshComponent* emptyCasingmesh;

	//to use for the UI ammo mod menu
	//this will allow the player to visualise what type of bullet they want with different properties and shieet
	UPROPERTY(VisibleAnywhere, Category = "AmmoMesh")
	UStaticMeshComponent* bulletMesh;

	//maybe make an enum for the different bullet types


	//allows the player to customise their bullet properties to suit what they want
	UPROPERTY(EditDefaultsOnly, Category = "AmmoProperties")
	float powderAmount;

	UPROPERTY(EditDefaultsOnly, Category = "AmmoProperties")
	float bulletGrain;

	//this will also affect recoil

	//using these values make a somewhat realistic output value?
	//could do in fps, joules and give it maths to calculate the overal properties below?


	//depending on the different properties this affects the damage and penetration
	//this will then later be on used for taking off body parts
	UPROPERTY(BlueprintReadOnly, Category = "AmmoProperties")
	float penetrationPower;

	UPROPERTY(BlueprintReadOnly, Category = "AmmoProperties")
	float fleshDamage;

	//could be a cool idea?
	UPROPERTY(BlueprintReadOnly, Category = "AmmoProperties")
	float boneBraking;


	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
