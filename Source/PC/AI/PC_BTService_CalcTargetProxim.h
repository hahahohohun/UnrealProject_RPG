// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PC_BTService_CalcTargetProxim.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BTService_CalcTargetProxim : public UBTService
{
	GENERATED_BODY()

public:
	UPC_BTService_CalcTargetProxim();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Behavior)
	float UnderRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Behavior)
	float NearRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Behavior)
	float MiddleRange = 0.f;
};
