#include "PC_HPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

UPC_HPBarWidget::UPC_HPBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxHp = -1.0f;
}

void UPC_HPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ensure(PB_HP);
	ensure(TB_HP);

	IPC_CharacterWidgetInterface* CharacterWidget = Cast<IPC_CharacterWidgetInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}
}

void UPC_HPBarWidget::UpdateHpBar(float NewCurrentHp, float NewMaxHp)
{
	CurrentHp = NewCurrentHp;
	MaxHp = NewMaxHp;

	if (MaxHp <= 0.f)
		return;
	
	if (PB_HP)
	{
		PB_HP->SetPercent(CurrentHp / MaxHp);
	}

	if (TB_HP)
	{
		TB_HP->SetText(FText::FromString(GetHPText()));
	}
}

FString UPC_HPBarWidget::GetHPText()
{
	return FString::Printf(TEXT("%.0f/%0.f"), CurrentHp, MaxHp);
}
