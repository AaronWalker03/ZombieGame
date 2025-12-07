// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCustomisationStruct.generated.h"

USTRUCT(BlueprintType)
struct FPlayerCustomisationData
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> footWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> legWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> tshirtWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> jacketWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> faceWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftClassPtr<AWeapon> weaponClass;

    UPROPERTY(BlueprintReadWrite)
    int playerLevel = 1;

    UPROPERTY(BlueprintReadWrite)
    int currentXP = 0;
};