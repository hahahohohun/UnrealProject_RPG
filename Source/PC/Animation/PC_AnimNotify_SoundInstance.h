// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PC/PC_Enum.h"
#include "PC_AnimNotify_SoundInstance.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_AnimNotify_SoundInstance : public UAnimNotify 
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere)
	USoundBase* SFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SFXSocketName = TEXT("root");
};
