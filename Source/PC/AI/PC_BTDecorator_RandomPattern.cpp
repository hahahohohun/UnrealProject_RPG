// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_BTDecorator_RandomPattern.h"

#include "PC_BTDecorator_CheckRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTDecorator_RandomPattern::UPC_BTDecorator_RandomPattern()
{
	NodeName = TEXT("RandomPattern Node");
}

bool UPC_BTDecorator_RandomPattern::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	uint32 RandomValue = FMath::RandRange(0, 100);
	if(RandomValue <= Percent)
	{
		return true;
	}
	
	return false;
}
