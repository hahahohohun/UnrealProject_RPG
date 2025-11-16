// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
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
	
	//
	
	UFUNCTION(BlueprintCallable)
	void OnFoliageChanged(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void OnShadowChanged(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void OnAAChanged(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void OnPostProcessChanged(bool bIsChecked);
	//
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* FoliageTextBlock;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShadowTextBlock;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AATextBlock;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* PostProcessTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	UCheckBox* FoliageCheckBox;

	UPROPERTY(meta=(BindWidget))
	UCheckBox* ShadowCheckBox;

	UPROPERTY(meta=(BindWidget))
	UCheckBox* AACheckBox;

	UPROPERTY(meta=(BindWidget))
	UCheckBox* PostProcessCheckBox;
	
protected:
	virtual void NativeConstruct() override;

	void InitializeGraphicSettings();

};
