// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_OptionSettingWidget.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "PC/Subsystem/PC_OptionSubsystem.h"
#include "PC/Subsystem/PC_UISubsystem.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_OptionSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//그래픽
	if (Title1TextBlock)
		Title1TextBlock->SetText(FText::FromString(TEXT("Graphic")));

	if (FoliageTextBlock)
		FoliageTextBlock->SetText(FText::FromString(TEXT("Foliage")));

	if (ShadowTextBlock)
		ShadowTextBlock->SetText(FText::FromString(TEXT("Shadow")));

	if (AATextBlock)
		AATextBlock->SetText(FText::FromString(TEXT("Anti-Aliasing")));

	if (PostProcessTextBlock)
		PostProcessTextBlock->SetText(FText::FromString(TEXT("Post Process")));


	if (FoliageCheckBox)
		FoliageCheckBox->OnCheckStateChanged.AddDynamic(this, &UPC_OptionSettingWidget::OnFoliageChanged);

	if (ShadowCheckBox)
		ShadowCheckBox->OnCheckStateChanged.AddDynamic(this, &UPC_OptionSettingWidget::OnShadowChanged);

	if (AACheckBox)
		AACheckBox->OnCheckStateChanged.AddDynamic(this, &UPC_OptionSettingWidget::OnAAChanged);

	if (PostProcessCheckBox)
		PostProcessCheckBox->OnCheckStateChanged.AddDynamic(this, &UPC_OptionSettingWidget::OnPostProcessChanged);

	//사운드
	if (Title2TextBlock)
		Title2TextBlock->SetText(FText::FromString(TEXT("Sound")));

	if (BGMSoundTextBlock)
		BGMSoundTextBlock->SetText(FText::FromString(TEXT("BGM")));

	if (FXSoundTextBlock)
		FXSoundTextBlock->SetText(FText::FromString(TEXT("FX")));

	if (FXSlider)
	{
		FXSlider->OnValueChanged.AddDynamic(
			this, &UPC_OptionSettingWidget::OnFXSoundValueChanged
		);
	}

	if (BGMSlider)
	{
		BGMSlider->OnValueChanged.AddDynamic(
			this, &UPC_OptionSettingWidget::OnBGMSoundValueChanged
		);
	}

	//카메라
	if (Title3TextBlock)
		Title3TextBlock->SetText(FText::FromString(TEXT("Camera")));

	if (CameraRotTextBlock)
		CameraRotTextBlock->SetText(FText::FromString(TEXT("sensitivity")));

	if (CameraSlider)
	{
		CameraSlider->OnValueChanged.AddDynamic(
			this, &UPC_OptionSettingWidget::OnCameraRotValueChanged
		);
	}

	//Save , Cancel
	if (SaveTextBlock)
		SaveTextBlock->SetText(FText::FromString("Save"));

	if (CloseTextBlock)
		CloseTextBlock->SetText(FText::FromString("Close"));

	if (SaveButton)
		SaveButton->OnClicked.AddDynamic(this, &UPC_OptionSettingWidget::UPC_OptionSettingWidget::SaveSetting);

	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UPC_OptionSettingWidget::UPC_OptionSettingWidget::OnClosed);


	//reset
	if (ResetTextBlock)
		ResetTextBlock->SetText(FText::FromString("Reset"));

	if (ResetButton)
		ResetButton->OnClicked.AddDynamic(this, &UPC_OptionSettingWidget::UPC_OptionSettingWidget::RestSetting);
	
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPC_OptionSubsystem* subsystem = GameInstance->GetSubsystem<UPC_OptionSubsystem>())
		{
			OptionSubsystem = subsystem;
		}
	}

	if (OptionSubsystem.IsValid())
	{
		RefreshSetting();
	}
}

void UPC_OptionSettingWidget::InitializeGraphicSettings()
{
	if (FoliageCheckBox)
		FoliageCheckBox->SetIsChecked(ApplyOption.bUseFoliage);

	if (ShadowCheckBox)
		ShadowCheckBox->SetIsChecked(ApplyOption.bUseShadow);

	if (AACheckBox)
		AACheckBox->SetIsChecked(ApplyOption.bUseAA);

	if (PostProcessCheckBox)
		PostProcessCheckBox->SetIsChecked(ApplyOption.bUsePostProcess);
}

void UPC_OptionSettingWidget::InitializeSoundSettings()
{
	OnFXSoundValueChanged(ApplyOption.SFXVolume);
	if (FXSlider)
		FXSlider->SetValue(ApplyOption.SFXVolume);

	OnBGMSoundValueChanged(ApplyOption.BGMVolume);
	if (BGMSlider)
		BGMSlider->SetValue(ApplyOption.BGMVolume);
}

void UPC_OptionSettingWidget::InitializeCameraSettings()
{
	OnCameraRotValueChanged(ApplyOption.MouseSensitivity);
	if (CameraSlider)
		CameraSlider->SetValue(ApplyOption.MouseSensitivity);
}

void UPC_OptionSettingWidget::OnOpened()
{
	bIsActive = true;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);

		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetIgnoreLookInput(true);
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetPause(true);
	}
}

void UPC_OptionSettingWidget::OnClosed()
{
	bIsActive = false;
	SetVisibility(ESlateVisibility::Collapsed);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		// 다시 게임 전용 입력 모드로
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);

		PlayerController->bShowMouseCursor = false;
		PlayerController->SetIgnoreLookInput(false);
		PlayerController->SetIgnoreMoveInput(false);

		PlayerController->SetPause(false);
	}
}

void UPC_OptionSettingWidget::SaveSetting()
{
	OptionSubsystem->ApplyAndSaveOption(ApplyOption);

	//카메라
	if(APlayerController* PlayerController = GetOwningPlayer())
		OptionSubsystem->ApplyCameraOptions(PlayerController);
}

void UPC_OptionSettingWidget::RefreshSetting()
{
	ApplyOption = OptionSubsystem->GetCurrentOption();

	InitializeGraphicSettings();
	InitializeSoundSettings();
	InitializeCameraSettings();
}

void UPC_OptionSettingWidget::RestSetting()
{
	OptionSubsystem->RestOption();
	RefreshSetting();
}

void UPC_OptionSettingWidget::OnFoliageChanged(bool bIsChecked)
{
	ApplyOption.bUseFoliage = bIsChecked;
}

void UPC_OptionSettingWidget::OnShadowChanged(bool bIsChecked)
{
	ApplyOption.bUseShadow = bIsChecked;
}

void UPC_OptionSettingWidget::OnAAChanged(bool bIsChecked)
{
	ApplyOption.bUseAA = bIsChecked;
}

void UPC_OptionSettingWidget::OnPostProcessChanged(bool bIsChecked)
{
	ApplyOption.bUsePostProcess = bIsChecked;
}

void UPC_OptionSettingWidget::OnCameraRotValueChanged(float Value)
{
	if (CameraRotValueTextBlock)
	{
		int32 IntValue = FMath::RoundToInt(Value);

		FString DisplayText = FString::FromInt(IntValue);
		CameraRotValueTextBlock->SetText(FText::FromString(DisplayText));

		ApplyOption.MouseSensitivity = IntValue;
	}
}

void UPC_OptionSettingWidget::OnFXSoundValueChanged(float Value)
{
	if (FXValueTextBlock)
	{
		int32 IntValue = FMath::RoundToInt(Value);

		FString DisplayText = FString::FromInt(IntValue);
		FXValueTextBlock->SetText(FText::FromString(DisplayText));

		ApplyOption.SFXVolume = IntValue;
	}
}

void UPC_OptionSettingWidget::OnBGMSoundValueChanged(float Value)
{
	if (BGMValueTextBlock)
	{
		int32 IntValue = FMath::RoundToInt(Value);

		FString DisplayText = FString::FromInt(IntValue);
		BGMValueTextBlock->SetText(FText::FromString(DisplayText));
		
		ApplyOption.BGMVolume = IntValue;
	}
}
