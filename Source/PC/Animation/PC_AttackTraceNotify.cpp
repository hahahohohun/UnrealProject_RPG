// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_AttackTraceNotify.h"

#include "PC/Interface/PC_AnimationAttackInterface.h"

void UPC_AttackTraceNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if(MeshComp)
	{
		if(IPC_AnimationAttackInterface* AttackInterface = Cast<IPC_AnimationAttackInterface>(MeshComp->GetOwner()))
		{
			AttackInterface->AttackTrace(isStart, TraceBoneName);
		}
	}
}
