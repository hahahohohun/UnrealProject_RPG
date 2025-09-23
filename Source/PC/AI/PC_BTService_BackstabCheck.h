// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PC_BTService_BackstabCheck.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTService_BackstabCheck : public UBTService
{
	GENERATED_BODY()

public:
	UPC_BTService_BackstabCheck();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 100; //

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAttackIndicator = false;
};
