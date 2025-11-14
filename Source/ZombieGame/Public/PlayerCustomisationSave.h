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
    TSoftObjectPtr<USkeletalMesh> footWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> legWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> tshirtWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> jacketWearMesh;

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> faceWearMesh;

};
