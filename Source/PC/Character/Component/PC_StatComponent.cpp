// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_StatComponent.h"

#include "Kismet/GameplayStatics.h"

#include "PC/Character/PC_BaseCharacter.h"
#include "PC/Utills/PC_GameUtill.h"

// Sets default values for this component's properties
UPC_StatComponent::UPC_StatComponent()
{
	bWantsInitializeComponent = true;

}

void UPC_StatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (APC_BaseCharacter* CharacterBase = Cast<APC_BaseCharacter>(GetOwner()))
	{
		if (FPC_CharacterStatTableRow* StatRow = FPC_GameUtil::GetCharacterStatData(CharacterBase->CharacterType))
			BaseStat = *StatRow;
	}
	
	ResetStats();
}

void UPC_StatComponent::AddBaseStat(const FPC_CharacterStatTableRow& InAddBaseStat)
{
	BaseStat = BaseStat + InAddBaseStat; 
	OnStatChangedDelegate.Broadcast(GetBaseStat(), GetModifierStat());
}

void UPC_StatComponent::SetBaseStat(const FPC_CharacterStatTableRow& InBaseStat)
{
	BaseStat = InBaseStat;
	OnStatChangedDelegate.Broadcast(GetBaseStat(), GetModifierStat()); 
}

void UPC_StatComponent::SetModifierStat(const FPC_CharacterStatTableRow& InModifierStat)
{
	ModifierStat = InModifierStat;
	OnStatChangedDelegate.Broadcast(GetBaseStat(), GetModifierStat());
}

void UPC_StatComponent::HealHp(float InHealAmount)
{
	CurrentHp = FMath::Clamp(CurrentHp + InHealAmount, 0, GetTotalStat().MaxHp);
	OnHPChangedDelegate.Broadcast(CurrentHp, MaxHp);
}

float UPC_StatComponent::ApplyDamage(float InDamage)
{
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);

	SetHp(PrevHp - ActualDamage);
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnCharacterDieDelegate.Broadcast();
	}

	return ActualDamage;
}

void UPC_StatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHp);
	
	OnHPChangedDelegate.Broadcast(CurrentHp, MaxHp);
}

void UPC_StatComponent::ResetStats()
{
	MaxHp = BaseStat.MaxHp;
	SetHp(MaxHp);
}
