// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_JumpToNextAttackMontageNotify.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

void UPC_JumpToNextAttackMontageNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);


	APawn* ControllingPawn = Cast<APawn>(MeshComp->GetOwner());
	if(!ControllingPawn) return;

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if(!AIPawn) return;

	AIPawn->JumpToNextAttackMontage();
}
