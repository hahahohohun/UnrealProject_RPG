// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_StatComponent.h"

#include "Kismet/GameplayStatics.h"

#include "PC/Character/PC_BaseCharacter.h"

// Sets default values for this component's properties
UPC_StatComponent::UPC_StatComponent()
{

}

void UPC_StatComponent::AddBaseStat(const FPC_CharacterStatTableRow& InAddBaseStat)
{
}

void UPC_StatComponent::SetBaseStat(const FPC_CharacterStatTableRow& InSetBaseStat)
{
}

void UPC_StatComponent::SetModfierStat(const FPC_CharacterStatTableRow& InModfierStat)
{
}

void UPC_StatComponent::HealHp(float InHealAmount)
{
}

float UPC_StatComponent::ApplyDamage(float InDamage)
{
	return false;
	
}

void UPC_StatComponent::SetHp(float NewHp)
{

}

void UPC_StatComponent::ResetStats()
{
}
