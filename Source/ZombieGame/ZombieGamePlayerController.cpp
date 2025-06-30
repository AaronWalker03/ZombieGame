// Copyright Epic Games, Inc. All Rights Reserved.


#include "ZombieGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "ZombieGameCameraManager.h"

AZombieGamePlayerController::AZombieGamePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AZombieGameCameraManager::StaticClass();
}

void AZombieGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}
