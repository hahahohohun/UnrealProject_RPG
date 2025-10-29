// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_CamShakeNotify.h"

#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_CamShakeNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if(Magnitude != EPC_CameraShakeMagnitudeType::None)
	{
		FPC_GameUtil::CameraShake(Magnitude);
	}
}
