// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC/Data/PC_TableRows.h"
#include "PC_StatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FPC_OnHPChangedDelegate, float CurrentHP, float MaxHP);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPC_OnMPChangedDelegate, float CurrentMP, float MaxMP);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPC_OnStaminaChangedDelegate, float CurrentStamina, float MaxStamina);

DECLARE_MULTICAST_DELEGATE(FPC_OnDeadDelegate);
//DECLARE_MULTICAST_DELEGATE_OneParam(FPC_OnDeadDelegate, EPC_DeadType DeadType);

DECLARE_MULTICAST_DELEGATE_TwoParams(FPC_OnSatChangedDelegate, const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModfierStat);

USTRUCT()
struct FPC_CharacterStatModifier
{
	GENERATED_BODY()

	UPROPERTY()
	FPC_CharacterStatTableRow AddStat; // 가산

	UPROPERTY()
	FPC_CharacterStatTableRow MulStat; // 승산
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PC_API UPC_StatComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	// Sets default values for this component's properties
	UPC_StatComponent();
	virtual  void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FPC_OnHPChangedDelegate OnHPChangedDelegate;
	FPC_OnMPChangedDelegate OnMPChangedDelegate;
	FPC_OnStaminaChangedDelegate OnStaminaChangedDelegate;
	
	FPC_OnSatChangedDelegate OnStatChangedDelegate;
	FPC_OnDeadDelegate OnCharacterDieDelegate;

	void AddBaseStat(const FPC_CharacterStatTableRow& InAddBaseStat);
	void SetBaseStat(const FPC_CharacterStatTableRow& InSetBaseStat);
	void SetModifierStat(const FPC_CharacterStatTableRow& InModifierStat);

	//버프형
	void AddStatusEffect(uint32 StatusEffectId);
	void RemoveStatusEffect(uint32 StatusEffectId);

	void RecalculateStats();
	//

	
	const FPC_CharacterStatTableRow& GetBaseStat() const { return BaseStat; }
	const FPC_CharacterStatTableRow& GetModifierStat() const { return ModifierStat; }
	FPC_CharacterStatTableRow GetTotalStat() const { return BaseStat + ModifierStat; }

	float GetCurrentHp() const { return CurrentHp; }
	float GetMaxHp() const { return MaxHp; }

	void HealHp(float InHealAmount);
	void AddStamina(float InAmount);
	float ApplyDamage(float InDamage, AActor* DamageCauser, bool SpawnEffect);
	bool TryConsumeStamina(float InAmount);
	void ConsumeStamina(float InAmount);
	
	void SetHp(float NewHp);
	float MaxHp;
	float CurrentHp;

	void SetStamina(float newStamina);
	float MaxStamina;
	float CurrentStamina;
	float StaminaRegenCooldown = 0.0f;
	
	UPROPERTY(Meta= (AllowPrivateAccess = "true"))
	FPC_CharacterStatTableRow BaseStat;

	UPROPERTY(Meta= (AllowPrivateAccess = "true"))
	FPC_CharacterStatTableRow ModifierStat;

	void ResetStats();

private:
	TMap<uint32, FPC_CharacterStatModifier> ActiveStatusEffectModifiers;

	//스텟 최종치
	FPC_CharacterStatTableRow CachedFinal;
	
};


