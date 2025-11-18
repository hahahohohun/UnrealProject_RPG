// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PC/PC_Enum.h"
#include "NiagaraSystem.h"
#include "PC_WeaponSparkEffectNotify.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_WeaponSparkEffectNotify : public UAnimNotify 
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere)
	bool bRight = true;
	UPROPERTY(EditAnywhere)
	bool bStart = true;
};
