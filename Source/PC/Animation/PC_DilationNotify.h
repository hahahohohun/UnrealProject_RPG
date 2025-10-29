// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PC/PC_Enum.h"
#include "PC_DilationNotify.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_DilationNotify : public UAnimNotify 
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Dilation = 0;
};
