// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_BTDecorator_CheckRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTDecorator_CheckRange::UPC_BTDecorator_CheckRange()
{
	NodeName = TEXT("CheckRange Node");
}

bool UPC_BTDecorator_CheckRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!ControllingPawn)
		return false;

	APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (!Target)
		return false;
 
	const float Dis = ControllingPawn->GetDistanceTo(Target);
	return Range >= Dis;
}
