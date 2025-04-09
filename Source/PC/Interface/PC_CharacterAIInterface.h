// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PC_CharacterAIInterface.generated.h"

DECLARE_DELEGATE(FAICharacterAttackFinished);

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
	virtual float GetAIPatrolRadius() = 0;
	virtual float GetAIDetectRange() = 0;
	virtual float GetAIAttackRange() = 0;
	virtual float GetAITurnSpeed() = 0;

	virtual void SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished) = 0;
	virtual void Attack() = 0;
};
