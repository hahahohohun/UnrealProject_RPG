// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PC_CharacterDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FPC_WeaponData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintType, EditAnywhere)
	int32 WeaponId_L =-1;

	UPROPERTY(BlueprintType, EditAnywhere)
	int32 WeaponId_R = -1;
	
};

UCLASS()
class PC_API UPC_CharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> HitFx; //피격 당했을떄

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> GuardFx;
	
	UPROPERTY(EditAnywhere)
	FName WeaponSocketName_L;

	UPROPERTY(EditAnywhere)
	FName WeaponSocketName_R;

	UPROPERTY(EditAnywhere)
	TArray<FPC_WeaponData> WeaponIds;

	//맞았을때 반짝
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> DamgeMaterial = nullptr;
};
