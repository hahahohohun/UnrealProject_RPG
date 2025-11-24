// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC/Character/Component/PC_BattleComponent.h"
#include "PC/Character/Component/PC_CrowdControlComponent.h"
#include "PC/Character/Component/PC_SkillComponent.h"
#include "PC/Character/Component/PC_StatComponent.h"
#include "PC/Character/Component/PC_StatusEffectComponent.h"
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
	virtual void WeaponSparkEffect(bool bStart, bool bRight) = 0;
	
	virtual void AttackTraceWithWeapon(bool bStart, bool bRight, bool PowerAttack) = 0;
	virtual void OnApplyStatusEffect(uint32 StatusEffectID) = 0;
	virtual void ReactAttackBreak() = 0;
	
	virtual void LaunchCharacter(FVector StartPos, FVector CauserPos, float Power) = 0;
	virtual  UStaticMeshComponent* GetWeapon_L_StaticMeshComponent() = 0;
	virtual  UStaticMeshComponent* GetWeapon_R_StaticMeshComponent() = 0;

	virtual UPC_CrowdControlComponent* GetCrowdControlComponent() = 0;

	virtual UPC_SkillComponent* GetSkillComponent() const = 0;
	virtual UPC_BattleComponent* GetBattleComponent() const = 0;
	virtual UPC_StatComponent* GetStatComponent() const = 0;

	virtual UPC_StatusEffectComponent* GetStatusEffectComponent() const = 0;

	virtual TPair<FName, FName> GetWeaponTraceNames(bool bRight) = 0;

	virtual UPC_CharacterDataAsset* GetCharacterDataAsset() = 0;
	virtual bool IsDead() = 0;
	
	virtual FPC_OnStartSkillDelegate& GetOnStartSkillDelegate() =0;
	virtual FPC_OnEndSkillDelegate& GetOnEndSkillDelegate() =0;

	//
	virtual bool IsRolling() = 0;
	virtual bool IsGuarding(FVector ImpactPoint) = 0;
};
