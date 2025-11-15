// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_OptionSettingWidget.h"

void UPC_OptionSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPC_OptionSettingWidget::OnOpened()
{
	bIsActive = true;
}

void UPC_OptionSettingWidget::OnClosed()
{
	bIsActive = false;
}
