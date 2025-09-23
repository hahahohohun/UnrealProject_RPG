// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HLSLTypeAliases.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PC_BTTask_UseSkill.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTTask_UseSkill : public UBTTaskNode
{
	GENERATED_BODY()

	public:
	UPC_BTTask_UseSkill();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void OnEndSkill(uint32 InSkillId);

	UPROPERTY(EditAnywhere)
	uint32 SkillId;

	TWeakObjectPtr<ACharacter> OwenrCharacter = nullptr;
	TWeakObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent = nullptr;
};



