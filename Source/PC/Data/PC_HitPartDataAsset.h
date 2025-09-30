// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PC_HitPartDataAsset.generated.h"

class UInputAction;

UCLASS()
class PC_API UPC_HitPartDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FString> HeadKeywords;

	UPROPERTY(EditAnywhere)
	TArray<FString> BodyKeywords;

	UPROPERTY(EditAnywhere)
	TArray<FString> ArmKeywords;

	UPROPERTY(EditAnywhere)
	TArray<FString> LegKeywords;

	UPROPERTY(EditAnywhere)
	TArray<FString> LeftMarkers;

	UPROPERTY(EditAnywhere)
	TArray<FString> RightMarkers;
};
