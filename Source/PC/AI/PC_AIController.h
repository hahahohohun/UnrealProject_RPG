// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "PC_AIController.generated.h"

/**
 * 
 */

struct FPC_EnemyTableRow;
class UAISenseConfig_Damage;
class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;

UCLASS()
class PC_API APC_AIController : public AAIController
{
	GENERATED_BODY()

public:
	APC_AIController();
	
	void RunAI();
	void StopAI();

	virtual void OnPossess(APawn* Possessed) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	
	FPC_EnemyTableRow* GetEnemyData();

protected:

	UFUNCTION()
	void OnPerceptionUpdate(const TArray<AActor*>& UpdatedActors);
	
	void SetupSenseConfig();
	
	void HandleSensedSight(AActor* InActor);
	void HandleSensedHearing(AActor* InActor, FVector InLocation);
	void HandleSensedDamage(AActor* InActor);

	void HandleLoseTarget(AActor* InActor);

	FAIStimulus GetAIStimulus(AActor* Actor, EPC_AISenseType AIPerceptionSense);

private:
	UPROPERTY()
	TObjectPtr<UBlackboardData> BBAsset;

	UPROPERTY()
	TObjectPtr<UBehaviorTree> BTAsset;
	
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightSense = nullptr;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingSense = nullptr;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Damage> DamageSense = nullptr;
};

