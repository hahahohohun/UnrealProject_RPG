// Fill out your copyright notice in the Description page of Project Settings.


#include "PC/Subsystem/PC_UISubsystem.h"

#include "PC/UI/PC_HPBarWidget.h"
#include "PC/UI/PC_HUDWidget.h"
#include "PC/UI/PC_StatusEffectWidget.h"

UPC_UISubsystem::UPC_UISubsystem()
{
	static ConstructorHelpers::FClassFinder<UPC_HUDWidget> HUDWidgetAsset(TEXT("/Game/ProjectClass/UI/WBP_HUDUI.WBP_HUDUI_C"));
	if (HUDWidgetAsset.Succeeded())
	{
		HUDWidgetClass = HUDWidgetAsset.Class;
	}

	static ConstructorHelpers::FClassFinder<UPC_HPBarWidget> HPBarWidgetAsset(TEXT("/Game/ProjectClass/UI/WBP_HPBar.WBP_HPBar_C"));
	if(HPBarWidgetAsset.Succeeded())
	{
		HPBarWidgetClass = HPBarWidgetAsset.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UPC_DamageFloaterWidget> DamageFloaterWidgetAsset(TEXT("/Game/ProjectClass/UI/WBP_DamageFloater.WBP_DamageFloater_C"));
	if(DamageFloaterWidgetAsset.Succeeded())
	{
		DamageFloaterWidgetClass = DamageFloaterWidgetAsset.Class;
	}
	
	//static ConstructorHelpers::FClassFinder<upc_dam>
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

UPC_DamageFloaterWidget* UPC_UISubsystem::CreateDamageFloater(AActor* Owner)
{
	if(UPC_DamageFloaterWidget* Widget = Cast<UPC_DamageFloaterWidget>(CreateUI(DamageFloaterWidgetClass.Get())))
	{
		Widget->AddToViewport();
		Widget->SetOwningActor(Owner);

		return Widget;
	}

	return nullptr;
}
