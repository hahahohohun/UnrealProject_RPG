// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PC_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PC_API APC_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
	protected:
    	virtual void PostInitializeComponents() override;
    	virtual void BeginPlay() override;
    	virtual void OnPossess(APawn* InPawn) override;
};
