// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PC_AnimationAttackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPC_AnimationAttackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PC_API IPC_AnimationAttackInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual  void AttackTrace(bool bStart, FName TraceBoneName);
};
