// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_StatComponent.h"

#include "Kismet/GameplayStatics.h"

#include "PC/Character/PC_BaseCharacter.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

// Sets default values for this component's properties
UPC_StatComponent::UPC_StatComponent()
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UPC_StatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (APC_BaseCharacter* CharacterBase = Cast<APC_BaseCharacter>(GetOwner()))
	{
		if (FPC_CharacterStatTableRow* StatRow = FPC_GameUtil::GetCharacterStatData(CharacterBase->CharacterDataID))
			BaseStat = *StatRow;
	}
	
	ResetStats();
}

void UPC_StatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(const IPC_PlayerCharacterInterface* Interface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
	{
		const UPC_PlayerDataAsset* PlayerDataAsset = Interface->GetPlayerData();
		check(PlayerDataAsset);

		if(StaminaRegenCooldown > 0.0f)
			StaminaRegenCooldown -= DeltaTime;

		if(StaminaRegenCooldown <= 0.0f && PlayerDataAsset->StaminaRegenPerSec > 0.0f && CurrentStamina < MaxStamina)
		{
			SetStamina(FMath::Min(MaxStamina, CurrentStamina + PlayerDataAsset->StaminaRegenPerSec * DeltaTime));
		}
	}
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

bool UPC_StatComponent::TryConsumeStamina(float InAmount)
{
	if(InAmount <= 0.0f)
		return true;

	if(CurrentStamina < InAmount)
		return false;

	ConsumeStamina(InAmount);

	if(IPC_PlayerCharacterInterface* PlayerCharacterInterface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
	{
		const UPC_PlayerDataAsset* PlayerDataAsset = PlayerCharacterInterface->GetPlayerData();
		check(PlayerDataAsset);

		StaminaRegenCooldown = PlayerDataAsset->StaminaRegenDelay;
	}

	return true;
}

void UPC_StatComponent::ConsumeStamina(float InAmount)
{
	const float PrevStamina = CurrentStamina;
	const float ActualStamina = FMath::Clamp(InAmount, 0, CurrentStamina);

	SetStamina(PrevStamina - ActualStamina);
}

void UPC_StatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHp);
	
	OnHPChangedDelegate.Broadcast(CurrentHp, MaxHp);
}

void UPC_StatComponent::SetStamina(float NewStamina)
{
	CurrentStamina = FMath::Clamp<float>(NewStamina, 0.0f, MaxStamina);
	OnStaminaChangedDelegate.Broadcast(CurrentStamina, MaxStamina);
}

void UPC_StatComponent::ResetStats()
{
	MaxHp = BaseStat.MaxHp;
	SetHp(MaxHp);

	MaxStamina = BaseStat.MaxStamina;
	SetStamina(MaxStamina);
}
