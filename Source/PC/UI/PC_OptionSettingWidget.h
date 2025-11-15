// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "PC_OptionSettingWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_OptionSettingWidget : public UPC_UserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;
	
	void OnOpened();
	void OnClosed();
	
protected:
	virtual void NativeConstruct() override;


};
