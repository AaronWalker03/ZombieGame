// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerCustomisationSave.generated.h"


UCLASS()
class ZOMBIEGAME_API UPlayerCustomisationSave : public USaveGame
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadWrite)
    FString HeadMeshName;

    UPROPERTY(BlueprintReadWrite)
    FString TorsoMeshName;

    UPROPERTY(BlueprintReadWrite)
    FString LegsMeshName;

    UPROPERTY(BlueprintReadWrite)
    FString FeetMeshName;

    UPROPERTY(BlueprintReadWrite)
    FString WeaponName;

};
