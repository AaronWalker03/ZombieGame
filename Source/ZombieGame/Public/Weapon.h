// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class ZOMBIEGAME_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* mesh;

	//get weapon attachments and ajust for recoil
	//also depending on the bullet used this will affect the recoil
	//then assign somehwat random values for recoil 
	//maybe randomise bullet velocities which can affect recoil

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//ideally turn the current mesh into fully in depth to show components and what part can be modded

	

	// These get set by child classes
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	//use the bp
	TSubclassOf<AAmmunition> ammunitionType;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float magCapacity;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float currentAmmo;

	/*UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundBase* FireSound;*/

	float bulletVelocity;
	float energyJoules;

	float velocityms;
	float bulletMassKG;

	//could be a cool idea?
	/*UPROPERTY(BlueprintReadOnly, Category = "AmmoProperties")
	float boneBraking;*/

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void CalculateBallistics(float powder, float grain, float mm);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Shoot();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Aim();

	// Reload function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Reload();

};
