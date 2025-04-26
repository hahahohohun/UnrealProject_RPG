// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTDecorator_IsInAttackRange.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"


UPC_BTDecorator_IsInAttackRange::UPC_BTDecorator_IsInAttackRange()
{
	NodeName = TEXT("IsInAttackRange Node");
}

bool UPC_BTDecorator_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if(!ControllingPawn)
		return false;

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if(!AIPawn)
		return false;

	APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if(!Target)
		return false;

	const float DistanceToTarget = ControllingPawn->GetDistanceTo(Target);
	const float AttackRangeWithRadius = AIPawn->GetAIAttackRange();
	bResult = DistanceToTarget <= AttackRangeWithRadius;

	return bResult;
}
