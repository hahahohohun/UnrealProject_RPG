#include "PC_IndicatorWidget.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

void UPC_IndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	IPC_CharacterWidgetInterface* CharacterWidget = Cast<IPC_CharacterWidgetInterface>(OwningActor);
	if(CharacterWidget)
	{
		CharacterWidget->SetupIndicatorWidget(this);
	}

	Image_LockOn2->SetVisibility(ESlateVisibility::Collapsed);
	Image_Assassinate->SetVisibility(ESlateVisibility::Collapsed);
}

UPC_IndicatorWidget::UPC_IndicatorWidget(const FObjectInitializer& ObjectInitializer)
{
	
}

void UPC_IndicatorWidget::ToggleLockOnImage(bool bEnable)
{
	if (bEnable)
		Image_LockOn2->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	else
		Image_LockOn2->SetVisibility(ESlateVisibility::Collapsed);
}

void UPC_IndicatorWidget::ToggleAssassinateImage(bool bEnable)
{
	if (bEnable)
		Image_Assassinate->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	else
		Image_Assassinate->SetVisibility(ESlateVisibility::Collapsed);
}

