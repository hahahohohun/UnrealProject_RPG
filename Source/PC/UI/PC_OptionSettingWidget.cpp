// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_OptionSettingWidget.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_OptionSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// --- 텍스트 설정 ---
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
	
	InitializeGraphicSettings();
}

void UPC_OptionSettingWidget::InitializeGraphicSettings()
{
	
}

void UPC_OptionSettingWidget::OnOpened()
{
	bIsActive = true;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 1. 오너 플레이어 컨트롤러 가져오기
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		// 2. 입력 모드를 UIOnly로 설정해서
		//    마우스/키보드 입력이 게임이 아니라 UI로만 가도록 함
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);

		PlayerController->SetInputMode(InputMode);

		// 3. 마우스 커서 보이게
		PlayerController->bShowMouseCursor = true;

		// 4. 혹시 남는 입력을 완전히 막고 싶으면 (카메라 회전, 이동 등)
		PlayerController->SetIgnoreLookInput(true);
		PlayerController->SetIgnoreMoveInput(true);

		// 필요하면 게임 일시정지까지
		// PC->SetPause(true);
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

		// 게임을 일시정지 했었다면 해제
		// PC->SetPause(false);
	}
}

void UPC_OptionSettingWidget::OnFoliageChanged(bool bIsChecked)
{
	FString Msg = FString::Printf(TEXT("OnFoliageChanged : %s"), bIsChecked ? TEXT("true") : TEXT("false"));
	FPC_GameUtil::AddOnScreenDebugMessage(Msg);
}

void UPC_OptionSettingWidget::OnShadowChanged(bool bIsChecked)
{
	FString Msg = FString::Printf(TEXT("OnShadowChanged : %s"), bIsChecked ? TEXT("true") : TEXT("false"));
	FPC_GameUtil::AddOnScreenDebugMessage(Msg);
}

void UPC_OptionSettingWidget::OnAAChanged(bool bIsChecked)
{
	FString Msg = FString::Printf(TEXT("OnAAChanged : %s"), bIsChecked ? TEXT("true") : TEXT("false"));
	FPC_GameUtil::AddOnScreenDebugMessage(Msg);
}

void UPC_OptionSettingWidget::OnPostProcessChanged(bool bIsChecked)
{
	FString Msg = FString::Printf(TEXT("OnPostProcessChanged : %s"), bIsChecked ? TEXT("true") : TEXT("false"));
	FPC_GameUtil::AddOnScreenDebugMessage(Msg);
}
