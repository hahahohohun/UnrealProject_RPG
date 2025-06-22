// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PC_CameraDataAsset.generated.h"


UCLASS()
class PC_API UPC_CameraDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	FRotator CameraRot = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float CameraFov = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float TargetArmLength = 0.f;
};
