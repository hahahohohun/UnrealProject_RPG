// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PC/PC_Enum.h"
#include "PC_AnimNotify_Footstep.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_AnimNotify_Footstep : public UAnimNotify 
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere)
	USoundBase* OverrideFootstepSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	FName FootSocketName = TEXT("foot_l");
};
