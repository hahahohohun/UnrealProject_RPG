// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PC_BTTask_SetFocus.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTTask_SetFocus : public UBTTaskNode
{
	GENERATED_BODY()

	public:
	UPC_BTTask_SetFocus();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	bool bFocus = false;
};



