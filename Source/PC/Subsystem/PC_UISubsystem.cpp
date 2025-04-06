// Fill out your copyright notice in the Description page of Project Settings.


#include "PC/Subsystem/PC_UISubsystem.h"

#include "PC/UI/PC_HUDWidget.h"

UPC_UISubsystem::UPC_UISubsystem()
{
	static ConstructorHelpers::FClassFinder<UPC_HUDWidget> HUDWidgetAsset(TEXT("/Game/ProjectClass/UI/WBP_HUDUI.WBP_HUDUI_C"));
	if (HUDWidgetAsset.Succeeded())
	{
		HUDWidgetClass = HUDWidgetAsset.Class;
	}
}

void UPC_UISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPC_UISubsystem::CreateHUD()
{
	if (UPC_HUDWidget* Widget = Cast<UPC_HUDWidget>(CreateUI(HUDWidgetClass.Get())))
	{
		Widget->AddToViewport();
		HUDWidget = Widget;
	}
}
