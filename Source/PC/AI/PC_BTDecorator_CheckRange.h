// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "PC_BTDecorator_CheckRange.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTDecorator_CheckRange : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UPC_BTDecorator_CheckRange();

	UPROPERTY(EditAnywhere)
	float Range = 0;
	
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
