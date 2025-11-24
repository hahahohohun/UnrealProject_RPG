// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_AnimNotify_WeaponSound.h"

#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterInterface.h"

void UPC_AnimNotify_WeaponSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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
		UPC_BattleComponent* BattleComponent = CI->GetBattleComponent();
		if(BattleComponent)
		{
			BattleComponent->PlayWeaponSwingSound();
		}
	}
}
