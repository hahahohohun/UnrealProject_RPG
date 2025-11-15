// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_OptionData.h"
#include "GameFramework/SaveGame.h"
#include "PC_OptionSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_OptionSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FPC_OptionData SavedOption;
};
