#include "PC_AttackIndicatorWidget.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

void UPC_AttackIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	IPC_CharacterWidgetInterface* CharacterWidget = Cast<IPC_CharacterWidgetInterface>(OwningActor);
	if(CharacterWidget)
	{
		CharacterWidget->SetupAttackIndicatorOnWidget(this);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

UPC_AttackIndicatorWidget::UPC_AttackIndicatorWidget(const FObjectInitializer& ObjectInitializer)
{
	
}

void UPC_AttackIndicatorWidget::ToggleActivation(bool bEnable)
{
	if (bEnable)
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	else
		SetVisibility(ESlateVisibility::Collapsed);
}

