// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC/Character/Component/PC_CrowdControlComponent.h"
#include "PC/Data/PC_CharacterDataAsset.h"
#include "UObject/Interface.h"
#include "PC_CharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPC_CharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PC_API IPC_CharacterInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool HasWeapon() = 0;
	virtual void AttackTrace(bool bStart, FName TraceStartBoneName, FName TraceEndBoneName) = 0;
	virtual void AttackTraceWithWeapon(bool bStart) = 0;

	virtual  UStaticMeshComponent* GetWeaponStaticMeshComponent() = 0;
	virtual UPC_CrowdControlComponent* GetCrowdControlComponent() = 0;
	virtual TPair<FName, FName> GetWeaponTraceNames() = 0;
	
	virtual  UPC_CharacterDataAsset* GetCharacterDataAsset() = 0;
	virtual  bool IsDead() = 0;
};
