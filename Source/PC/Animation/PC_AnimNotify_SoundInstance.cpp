// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_AnimNotify_SoundInstance.h"

#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_AnimNotify_SoundInstance::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
		return;

	if (!SFX)
		return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter)
		return;

	FVector Location = OwnerCharacter->GetActorLocation();
	if (!SFXSocketName.IsNone() && MeshComp->DoesSocketExist(SFXSocketName))
	{
		Location = MeshComp->GetSocketLocation(SFXSocketName);
	}
	FPC_GameUtil::PlaySFXAtLocation(OwnerCharacter, SFX, Location);
}
