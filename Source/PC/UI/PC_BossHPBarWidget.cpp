#include "PC_BossHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

UPC_BossHPBarWidget::UPC_BossHPBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), CurrentHp(0.f), MaxHp(-1.f)
{
	
}

void UPC_BossHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPC_BossHPBarWidget::Init(ACharacter* InTargetCharacter)
{
	//Target은 Boss
	if(IPC_CharacterWidgetInterface* CharacterWidgetInterface = CastChecked<IPC_CharacterWidgetInterface>(InTargetCharacter))
	{
		CharacterWidgetInterface->SetupCharacterWidget(this);
	}

	if(IPC_CharacterAIInterface* CharacterAIInterface = CastChecked<IPC_CharacterAIInterface>(InTargetCharacter))
	{
		FPC_EnemyTableRow* EnemyTableRow = CharacterAIInterface->GetEnemyData();
		check(EnemyTableRow);

		if(TB_Name)
		{
			TB_Name->SetText(FText::FromString(EnemyTableRow->Name.ToString()));
		}
	}
}

void UPC_BossHPBarWidget::UpdateHpBar(float NewCurrentHp, float NewMaxHp)
{
	if(MaxHp < 0.0f) //초기화
	{
		MaxHp = NewMaxHp;
		CurrentHp = NewCurrentHp;
	}

	if(NewCurrentHp < CurrentHp)
	{
		float DamageAmount = NewMaxHp - CurrentHp;
	}

	CurrentHp = NewCurrentHp;
	MaxHp = NewMaxHp;

	if(MaxHp <= 0.0f) return;

	if(PB_HP)
	{
		PB_HP->SetPercent(CurrentHp / MaxHp);
	}

	if(TB_HP)
	{
		TB_HP->SetText(FText::FromString(GetHPText()));
	}
}

FString UPC_BossHPBarWidget::GetHPText()
{
	return FString::Printf(TEXT("%.0f/%.0f"), CurrentHp, MaxHp);
}
