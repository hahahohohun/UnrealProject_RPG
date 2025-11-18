// Fill out your copyright notice in the Description page of Project Settings.


#include "PC/Character/Controller/PC_PlayerController.h"

#include "PC/Misc/GameMode/PCGameMode.h"
#include "PC/Subsystem/PC_UISubsystem.h"

class APCGameMode;

void APC_PlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void APC_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPC_UISubsystem* UISubsystem = GameInstance->GetSubsystem<UPC_UISubsystem>())
		{
			UISubsystem->CreateHUD();
		}
	}
}

void APC_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APC_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

