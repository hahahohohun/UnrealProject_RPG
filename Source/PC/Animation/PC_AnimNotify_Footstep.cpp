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
	
	USoundBase* FootSFX = OverrideFootstepSFX;

	if (!FootSFX)
	{
	    if (IPC_CharacterInterface* CI = Cast<IPC_CharacterInterface>(OwnerCharacter))
	    {
	        UPC_CharacterDataAsset* Data = CI->GetCharacterDataAsset();
	        FootSFX = Data ? Data->FootSFX : nullptr;
	    }
	}

	if (!FootSFX)
		return;

	if (OwnerCharacter->GetVelocity().Size2D() < 10.f) //가만히있으면 안나게
		return;
	
	FVector Location = OwnerCharacter->GetActorLocation();
	if (!FootSocketName.IsNone() && MeshComp->DoesSocketExist(FootSocketName))
	{
		Location = MeshComp->GetSocketLocation(FootSocketName);
	}

	FPC_GameUtil::PlaySFXAtLocation(OwnerCharacter, FootSFX, Location);
}
