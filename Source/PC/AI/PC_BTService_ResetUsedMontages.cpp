// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTService_ResetUsedMontages.h"
#include "AIController.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTService_ResetUsedMontages ::UPC_BTService_ResetUsedMontages()
{
	NodeName = TEXT("ResetUsedMontages");
	bNotifyBecomeRelevant = true;
}

void UPC_BTService_ResetUsedMontages::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!ControllingPawn)
	{
		return;
	}

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if (!AIPawn)
	{
		return;
	}

	AIPawn->ResetUsedMontage();
}
 