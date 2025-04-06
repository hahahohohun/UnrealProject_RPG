// Fill out your copyright notice in the Description page of Project Settings.


#include "PC/Character/Controller/PC_PlayerController.h"

#include "PC/Subsystem/PC_UISubsystem.h"

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

void APC_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}