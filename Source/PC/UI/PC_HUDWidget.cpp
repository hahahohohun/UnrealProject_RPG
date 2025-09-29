// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_HUDWidget.h"

#include "PC_CharacterStatWidget.h"
#include "Components/ProgressBar.h"
#include "PC/Interface/PC_CharacterHUDInterface.h"

void UPC_HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//WBP_HUDUI에 있는 WidgetCharacterStat 불러오기
	CharacterStatWidget = Cast<UPC_CharacterStatWidget>(GetWidgetFromName(TEXT("WidgetCharacterStat")));
	ensure(CharacterStatWidget);
	
	if(IPC_CharacterHUDInterface* HUDPawn = Cast<IPC_CharacterHUDInterface>(GetOwningPlayerPawn()))
	{
		HUDPawn->SetupHUDWidget(this);
	}

	if(PB_HP)
		PB_HP->SetPercent(1.f);
	
	if(PB_MP)
		PB_MP->SetPercent(1.f);
	
	if(PB_Stamina)
		PB_Stamina->SetPercent(1.f);
}

void UPC_HUDWidget::UpdateStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat)
{
	FPC_CharacterStatTableRow TotalStat = BaseStat + ModifierStat;
	CharacterStatWidget->UpdateStat(BaseStat, ModifierStat);
}

void UPC_HUDWidget::UpdateHPBar(float NewCurrentHP, float NewMaxHP)
{
	if(MaxHP < 0.f) //최초 초기화
	{
		MaxHP = NewMaxHP;
		CurrentHP = NewCurrentHP;
	}

	CurrentHP = NewCurrentHP;
	MaxHP = NewMaxHP;

	if(MaxHP <= 0.f) return;

	if(PB_HP)
	{
		float Value = CurrentHP / MaxHP;
		PB_HP->SetPercent(Value);
	}
}

void UPC_HUDWidget::UpdateStaminaBar(float NewCurrentStamina, float NewMaxStamina)
{
	
}

void UPC_HUDWidget::UpdateMPBar(float NewCurrentMP, float NewMaxMP)
{
	
}

void UPC_HUDWidget::OnEnCounterBossMonster(ACharacter* InCharacter)
{
	if(BossHPBar)
	{
		if(InCharacter)
		{
			BossHPBar->SetVisibility(ESlateVisibility::SelfHitTestInvisible); //보이지만 입력안받음
			//character -> boss
			BossHPBar->Init(InCharacter);
		}
		else
		{
			BossHPBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
