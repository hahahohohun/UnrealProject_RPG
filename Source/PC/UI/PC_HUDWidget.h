// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_BossHPBarWidget.h"
#include "PC_UserWidget.h"
#include "PC/Data/PC_TableRows.h"
#include "PC_HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_HUDWidget : public UPC_UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void UpdateStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat);

	void UpdateHPBar(float NewCurrentHP, float NewMaxHP);
	void UpdateStaminaBar(float NewCurrentStamina, float NewMaxStamina);
	void UpdateMPBar(float NewCurrentMP, float NewMaxMP);

	void OnEnCounterBossMonster(ACharacter* InCharacter); //boss 조우 했을때
protected:
	UPROPERTY()
	TObjectPtr<class UPC_CharacterStatWidget> CharacterStatWidget;

	UPROPERTY(meta=(BindWidgetOptional))
	class UProgressBar* PB_HP;
	
	UPROPERTY(meta=(BindWidgetOptional))
	class UProgressBar* PB_Stamina;

	UPROPERTY(meta=(BindWidgetOptional))
	class UProgressBar* PB_MP;

	UPROPERTY(meta=(BindWidgetOptional))
	UPC_BossHPBarWidget* BossHPBar;

	float CurrentHP;
	float MaxHP;
	
	float CurrentStamina;
	float MaxStamina;
	
	float CurrentMP;
	float MaxMP;
};
