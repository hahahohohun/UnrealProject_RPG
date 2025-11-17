// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_OptionData.h"
#include "Engine/DataAsset.h"
#include "PC_OptionConfigDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_OptionConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPC_OptionData DefaultOption;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPC_OptionData SaveOption;
};
