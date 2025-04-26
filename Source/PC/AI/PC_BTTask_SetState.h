// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PC/PC_Enum.h"
#include "PC_BTTask_SetState.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTTask_SetState : public UBTTaskNode
{
	GENERATED_BODY()

	public:
	UPC_BTTask_SetState();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	EPC_EnemyStateType StateType = EPC_EnemyStateType::None;
};



