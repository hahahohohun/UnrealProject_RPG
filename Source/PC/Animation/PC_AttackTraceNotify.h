// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PC_AttackTraceNotify.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_AttackTraceNotify : public UAnimNotify 
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere)
	bool isStart = false;

	UPROPERTY(EditAnywhere)
	FName TraceBoneName;
};
