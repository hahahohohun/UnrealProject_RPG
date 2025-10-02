// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PC/PC_Enum.h"
#include "PC_GameDataAsset.generated.h"


UCLASS()
class PC_API UPC_GameDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<EPC_CameraShakeMagnitudeType, UClass*> CameraShakeClass;
};
