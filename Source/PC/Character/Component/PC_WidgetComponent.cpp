// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_WidgetComponent.h"
#include "PC/UI/PC_UserWidget.h"

// Sets default values for this component's properties
UPC_WidgetComponent::UPC_WidgetComponent()
{
	
}

void UPC_WidgetComponent::InitWidget()
{
	Super::InitWidget();

	if(UPC_UserWidget* UserWidget = Cast<UPC_UserWidget>(GetWidget()))
	{
		UserWidget->SetOwningActor(GetOwner());
	}
}
