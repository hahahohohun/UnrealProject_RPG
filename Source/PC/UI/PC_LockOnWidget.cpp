#include "PC_LockOnWidget.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

void UPC_LockOnWidget::NativeConstruct()
{
	Super::NativeConstruct();

	IPC_CharacterWidgetInterface* CharacterWidget = Cast<IPC_CharacterWidgetInterface>(OwningActor);
	if(CharacterWidget)
	{
		CharacterWidget->SetupLockOnWidget(this);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

UPC_LockOnWidget::UPC_LockOnWidget(const FObjectInitializer& ObjectInitializer)
{
	
}

void UPC_LockOnWidget::ToggleActivation(bool bEnable)
{
	if (bEnable)
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	else
		SetVisibility(ESlateVisibility::Collapsed);
}

