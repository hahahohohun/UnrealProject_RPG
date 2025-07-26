// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_AttackTraceNotify.h"

#include "PC/Interface/PC_CharacterInterface.h"

void UPC_AttackTraceNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if(MeshComp)
	{
		if(IPC_CharacterInterface* AttackInterface = Cast<IPC_CharacterInterface>(MeshComp->GetOwner()))
		{
			AttackInterface->AttackTrace(isStart, TraceStartBoneName, TraceEndBoneName);
		}
	}
}
