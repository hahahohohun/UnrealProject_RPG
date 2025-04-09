// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PC_WidgetComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PC_API UPC_WidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()


public:
	// Sets default values for this component's properties
	UPC_WidgetComponent();

protected:
	virtual void InitWidget() override;
};
