// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Component/PC_BattleComponent.h"
#include "Component/PC_StatComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "PC/PC_Enum.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Interface/PC_AnimationAttackInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"
#include "PC_BaseCharacter.generated.h"


UCLASS(config=Game)
class APC_BaseCharacter : public ACharacter, public  IPC_AnimationAttackInterface , public IPC_CharacterWidgetInterface
{
	GENERATED_BODY()

public:
	APC_BaseCharacter();
	
	UPROPERTY(EditAnywhere)
	EPC_CharacterType CharacterType = EPC_CharacterType::None;
	
protected:
	
	virtual void AttackTrace(bool bStart, FName TraceBoneName) override;
	virtual void SetupCharacterWidget(UPC_UserWidget* InWidget) override;
	virtual void PostInitializeComponents() override;

protected:

	// To add mapping context
	virtual void BeginPlay();

public:
	void Applystat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_BattleComponent> BattleComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_StatComponent>  StatComponent = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPC_WidgetComponent> WidgetComponent = nullptr;
};

