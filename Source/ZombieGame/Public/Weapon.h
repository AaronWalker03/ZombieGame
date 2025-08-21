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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* Mesh;

	// These get set by child classes
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float totalAmmoCount;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float currentAmmoCount;

	/*UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USoundBase* FireSound;*/

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Shoot();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Aim();

	// Reload function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Reload();

};
