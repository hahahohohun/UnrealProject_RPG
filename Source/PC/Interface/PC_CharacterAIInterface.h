// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC/PC_Enum.h"
#include "UObject/Interface.h"
#include "PC_CharacterAIInterface.generated.h"

struct FPC_EnemyTableRow;
DECLARE_DELEGATE(FAICharacterAttackFinished);
//움직이 끝났을때 호출되는 델리게이트
DECLARE_DELEGATE(FAICharacterMoveMontageFinished);

DECLARE_DELEGATE(FAICharacterTurnFinished);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPC_CharacterAIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PC_API IPC_CharacterAIInterface
{
	GENERATED_BODY()

public:
	virtual FPC_EnemyTableRow* GetEnemyData() = 0;
	
	virtual float GetAIAttackRange() = 0;
	virtual float GetAITurnSpeed() = 0;

	virtual void ResetState() = 0;
	virtual void ChangeState(EPC_EnemyStateType StateType) = 0;
	virtual void SetDeadType(EPC_DeadType DeadType) = 0;
	
	virtual EPC_EnemyStateType GetState() = 0;
	virtual EPC_DeadType GetDeadType() = 0;

	virtual AActor* GetPatrolRoute() = 0;
	virtual void IncrementPatrolIndex() = 0;
	
	virtual void SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished) = 0;
	virtual void Attack(bool bLastAttack) = 0;

	virtual void SetAIMoveMontageFinishedDelegate(const FAICharacterMoveMontageFinished& InOnMoveMontageFinished) = 0;
	
	virtual void TurnInPlace(float TurnAnimDegree) = 0;
	
	virtual void DashBack()= 0;
	
	virtual void SetAITurnFinishDelegate(const FAICharacterTurnFinished& InOnTurnFinished) = 0;
	virtual void ResetUsedMontage() = 0;
	virtual void JumpToNextAttackMontage() = 0;

};
