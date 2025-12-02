// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_AnimNotify_Footstep.h"

#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_AnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp)
		return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter)
		return;
	
	if (IPC_CharacterInterface* CI = Cast<IPC_CharacterInterface>(OwnerCharacter))
	{
		CI->FootStepSound(OverrideFootstepSFX, FootSocketName);
	}
}
