// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PC_AIController.generated.h"

/**
 * 
 */
UCLASS()
class PC_API APC_AIController : public AAIController
{
	GENERATED_BODY()

public:
	void RunAI();
	void StopAI();

protected:
	virtual void OnPossess(APawn* Possessed);

private:
	UPROPERTY()
	TObjectPtr<UBlackboardData> BBAsset;

	UPROPERTY()
	TObjectPtr<UBehaviorTree> BTAsset;
	
	
};
