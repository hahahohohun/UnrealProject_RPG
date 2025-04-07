// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_HUDWidget.h"

#include "PC_CharacterStatWidget.h"
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
}

void UPC_HUDWidget::UpdateStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat)
{
	FPC_CharacterStatTableRow TotalStat = BaseStat + ModifierStat;
	CharacterStatWidget->UpdateStat(BaseStat, ModifierStat);
}
