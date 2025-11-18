#include "PC_HPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PC/Character/PC_NonPlayableCharacter.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_HPBarWidget::UPC_HPBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxHp = -1.0f;
}

void UPC_HPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ensure(PB_HP);
	ensure(TB_HP);

	if (TB_Debug)
		TB_Debug->SetVisibility(ESlateVisibility::Hidden);
}

void UPC_HPBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TB_Debug)
	{
		if (FPC_GameUtil::IsDebugDrawing(this))
		{
			if (TB_Debug->IsVisible() == false)
				TB_Debug->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

			if (OwningActor)
			{
				FString DebugMsg;
				if (IPC_CharacterInterface* Character = Cast<IPC_CharacterInterface>(OwningActor))
				{
					DebugMsg.Append(GetHPText());
				}

				if(APC_NonPlayableCharacter* NonPlayableCharacter = Cast<APC_NonPlayableCharacter>(OwningActor))
				{
					EPC_EnemyStateType State = NonPlayableCharacter->GetEnemyStateType();
					FString StateStr = UEnum::GetValueAsString(State);

					DebugMsg += LINE_TERMINATOR;        // 줄바꿈
					DebugMsg += TEXT("EnemyState: ");
					DebugMsg += StateStr;
				}
				
				TB_Debug->SetText(FText::FromString(DebugMsg));

			}
		}
		else
		{
			if (TB_Debug->IsVisible())
				TB_Debug->SetVisibility(ESlateVisibility::Collapsed);
		}
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
