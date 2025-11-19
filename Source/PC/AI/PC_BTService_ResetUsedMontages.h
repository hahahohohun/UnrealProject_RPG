// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PC_BTService_ResetUsedMontages.generated.h"

UCLASS()
class PC_API UPC_BTService_ResetUsedMontages : public UBTService
{
	GENERATED_BODY()

public:
	UPC_BTService_ResetUsedMontages ();
	
public:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
