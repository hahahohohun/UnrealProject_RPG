// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "PC/Subsystem/PC_OptionSubsystem.h"
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

	UFUNCTION()
	void OnClosed();

	UFUNCTION()
	void SaveSetting();

	UFUNCTION()
	void RefreshSetting();

	UFUNCTION()
	void RestSetting();
	//
	
	UFUNCTION(BlueprintCallable)
	void OnFoliageChanged(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void OnShadowChanged(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void OnAAChanged(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void OnPostProcessChanged(bool bIsChecked);

	UFUNCTION()
	void OnCameraRotValueChanged(float Value);

	UFUNCTION()
	void OnFXSoundValueChanged(float Value);

	UFUNCTION()
	void OnBGMSoundValueChanged(float Value);
	//

	UPROPERTY(meta=(BindWidget))
	UTextBlock* Title1TextBlock;
	
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


	//sound
	UPROPERTY(meta=(BindWidget))
	UTextBlock* Title2TextBlock;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* BGMSoundTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	USlider* BGMSlider;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* BGMValueTextBlock;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* FXSoundTextBlock;

	UPROPERTY(meta=(BindWidget))
	USlider* FXSlider;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* FXValueTextBlock;

	//Camera
	UPROPERTY(meta=(BindWidget))
	UTextBlock* Title3TextBlock;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CameraRotTextBlock;

	UPROPERTY(meta=(BindWidget))
	USlider* CameraSlider;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CameraRotValueTextBlock;
	
	
	//SAVE ,Cancel
	UPROPERTY(meta=(BindWidget))
	UTextBlock* SaveTextBlock;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CloseTextBlock;

	UPROPERTY(meta=(BindWidget))
	UButton* SaveButton;

	UPROPERTY(meta=(BindWidget))
	UButton* CloseButton;
	
	//Reset
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ResetTextBlock;

	UPROPERTY(meta=(BindWidget))
	UButton* ResetButton;


	
protected:
	virtual void NativeConstruct() override;

	void InitializeGraphicSettings();
	void InitializeSoundSettings();
	void InitializeCameraSettings();

private:
	TWeakObjectPtr<UPC_OptionSubsystem> OptionSubsystem;
	FPC_OptionData ApplyOption;
};
