// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PC_BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

	public:
	UPC_BTTask_Attack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};



