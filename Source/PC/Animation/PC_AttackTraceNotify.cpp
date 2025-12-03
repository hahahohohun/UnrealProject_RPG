// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_AttackTraceNotify.h"

#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_AttackTraceNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		if (IPC_CharacterInterface* AttackCharacter = Cast<IPC_CharacterInterface>(MeshComp->GetOwner()))
		{
			if (AttackCharacter->HasWeapon())
			{
				AttackCharacter->AttackTraceWithWeapon(bStart, bRight, PowerAttack, bWeaponSwingSound);
			}
			else
			{
				AttackCharacter->AttackTrace(bStart, TraceStartBoneName, TraceEndBoneName);
			}
		}
	}
}
