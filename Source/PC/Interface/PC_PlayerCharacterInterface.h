// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PC_PlayerCharacterInterface.generated.h"

class UPC_ActionComponent;
class UPC_BattleComponent;
class UPC_PlayerDataAsset;
class UPC_LockOnComponent;

UINTERFACE(MinimalAPI)
class UPC_PlayerCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

class PC_API IPC_PlayerCharacterInterface
{
	GENERATED_BODY()

public:
	virtual UStaticMeshComponent* GetWeaponStaticMeshComponent() const = 0;
	virtual UPC_ActionComponent* GetActionComponent() const = 0;
	virtual UPC_LockOnComponent* GetLockOnComponent() const = 0;
	virtual UPC_BattleComponent* GetBattleComponent() const = 0;
	virtual UPC_PlayerDataAsset* GetPlayerData() const = 0;
};
