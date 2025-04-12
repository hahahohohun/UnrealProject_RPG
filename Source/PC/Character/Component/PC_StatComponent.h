// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC/Data/PC_TableRows.h"
#include "PC_StatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChangedDelegate, float CurrentHP, float MaxHP);
DECLARE_MULTICAST_DELEGATE(FOnCharacterDieDelegate);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSatChangedDelegate, const FPC_CharacterStatTableRow& BaseStat,
                                     const FPC_CharacterStatTableRow& ModfierStat);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PC_API UPC_StatComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	// Sets default values for this component's properties
	UPC_StatComponent();
	virtual  void InitializeComponent() override;
	FOnHPChangedDelegate OnHPChangedDelegate;
	FOnSatChangedDelegate OnStatChangedDelegate;
	FOnCharacterDieDelegate OnCharacterDieDelegate;

	void AddBaseStat(const FPC_CharacterStatTableRow& InAddBaseStat);
	void SetBaseStat(const FPC_CharacterStatTableRow& InSetBaseStat);
	void SetModifierStat(const FPC_CharacterStatTableRow& InModifierStat);

	const FPC_CharacterStatTableRow& GetBaseStat() const { return BaseStat; }
	const FPC_CharacterStatTableRow& GetModifierStat() const { return ModifierStat; }
	FPC_CharacterStatTableRow GetTotalStat() const { return BaseStat + ModifierStat; }

	float GetCurrentHp() const { return CurrentHp; }
	float GetMaxHp() const { return MaxHp; }

	void HealHp(float InHealAmount);
	float ApplyDamage(float InDamage);

	void SetHp(float NewHp);

	float MaxHp;
	float CurrentHp;

	UPROPERTY(Meta= (AllowPrivateAccess = "true"))
	FPC_CharacterStatTableRow BaseStat;

	UPROPERTY(Meta= (AllowPrivateAccess = "true"))
	FPC_CharacterStatTableRow ModifierStat;

	void ResetStats();
};
