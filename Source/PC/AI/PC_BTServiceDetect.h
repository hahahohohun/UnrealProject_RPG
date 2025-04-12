// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PC_BTServiceDetect.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTServiceDetect : public UBTService
{
	GENERATED_BODY()
	

public:
	UPC_BTServiceDetect();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
