// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTTask_Attack.h"
#include "AIController.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTTask_Attack::UPC_BTTask_Attack()
{
	NodeName = TEXT("Attack Node");
}

EBTNodeResult::Type UPC_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* Pawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(Pawn);
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	FAICharacterAttackFinished OnAttackFinished;
	OnAttackFinished.BindLambda(
	[&]()
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	);
	
	AIPawn->SetAIAttackFinishDelegate(OnAttackFinished);
	AIPawn->Attack();
	return Result;
}
