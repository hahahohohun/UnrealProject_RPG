#include "PC_StatusEffectWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/PanelSlot.h"
#include "Components/TextBlock.h"
#include "PC/Character/Component/PC_StatusEffectComponent.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_StatusEffectWidget::UPC_StatusEffectWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UPC_StatusEffectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	IPC_CharacterWidgetInterface* CharacterWidget = Cast<IPC_CharacterWidgetInterface>(OwningActor);
	if(CharacterWidget)
	{
		CharacterWidget->SetupStatusEffectWidget(this);
	}
	
	GenerateFillOrder();
	Slots.Reset();
	for(int32 i = 0; i < MaxIcons; i++)
	{
		// 슬롯용 Overlay 생성 (아이콘 + 텍스트를 겹치기 위해)
		UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		
		if(UHorizontalBoxSlot* ImgSlot = StatusContainer->AddChildToHorizontalBox(Overlay))
		{
			ImgSlot->SetPadding(FMargin(2.f,0.f));
			ImgSlot->SetHorizontalAlignment(HAlign_Center);
			ImgSlot->SetVerticalAlignment(VAlign_Center);
		}
		// 아이콘 이미지
		UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Overlay->AddChildToOverlay(Img);

		// 시간 텍스트
		UTextBlock* TimeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Overlay->AddChildToOverlay(TimeText);
		TimeText->SetJustification(ETextJustify::Center);
		TimeText->SetVisibility(ESlateVisibility::Collapsed);

		FStatusEffectSlot StatusEffectSlot;
		StatusEffectSlot.Icon = Img;
		StatusEffectSlot.TimeText = TimeText;
		Img->SetVisibility(ESlateVisibility::Collapsed);
		Slots.Add(StatusEffectSlot);
	}
	IdToSlotIndex.Reset();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPC_StatusEffectWidget::UpdateStatusEffect(uint32 StatusEffectId, float RemainingTime)
{
	if (RemainingTime > KINDA_SMALL_NUMBER)
		AddOrUpdateStatusEffect(StatusEffectId, RemainingTime);
	else
		RemoveStatusEffect(StatusEffectId);
}

void UPC_StatusEffectWidget::AddOrUpdateStatusEffect(uint32 StatusEffectId, float RemainingTime)
{
    if (!IsValid(OwningActor))
        return;

    if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwningActor))
    {
        UPC_StatusEffectComponent* StatusEffectComponent = CharacterInterface->GetStatusEffectComponent();
        if (!StatusEffectComponent)
            return;

        FPC_StatusEffectInfo* EffectInfo = StatusEffectComponent->GetActiveStatusEffectInfo(StatusEffectId);
        if (!EffectInfo)
            return;

        FPC_StatusEffectTableRow* StatusEffectTableRow = FPC_GameUtil::GetStatusEffectData(StatusEffectId);
        check(StatusEffectTableRow);

        // 이미 슬롯이 있으면 그 슬롯 재사용
        if (int32* FoundIndex = IdToSlotIndex.Find(StatusEffectId))
        {
            const int32 Index = *FoundIndex;
            if (!Slots.IsValidIndex(Index))
                return;

            FStatusEffectSlot& StatusEffectSlot = Slots[Index];

            // 아이콘 갱신
            if (StatusEffectSlot.Icon)
            {
                FSlateBrush Brush;
                Brush.SetResourceObject(StatusEffectTableRow->Icon);
                Brush.ImageSize = FVector2D(45.f, 45.f);
                StatusEffectSlot.Icon->SetBrush(Brush);
                StatusEffectSlot.Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            }

            // 시간 텍스트 갱신
            if (StatusEffectSlot.TimeText)
            {
                // 예: 소수점 없애고 "3" 초만 보여주기
                const int32 RemainInt = FMath::CeilToInt(RemainingTime);
                StatusEffectSlot.TimeText->SetText(FText::AsNumber(RemainInt));
                StatusEffectSlot.TimeText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            }

            return;
        }

        // 빈 슬롯 찾기 (FillOrder를 쓰고 싶다면 여기서 사용)
        int32 FreeIndex = INDEX_NONE;
        for (int32 i = 0; i < Slots.Num(); ++i)
        {
            if (Slots[i].Icon && Slots[i].Icon->GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
            {
                FreeIndex = i;
                break;
            }
        }

        if (FreeIndex == INDEX_NONE)
            return;

        FStatusEffectSlot& FreeSlot = Slots[FreeIndex];

        // 아이콘 설정
        if (FreeSlot.Icon)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(StatusEffectTableRow->Icon);
            Brush.ImageSize = FVector2D(45.f, 45.f);
            FreeSlot.Icon->SetBrush(Brush);
            FreeSlot.Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }

        // 시간 텍스트 설정
        if (FreeSlot.TimeText)
        {
            const int32 RemainInt = FMath::CeilToInt(RemainingTime);
            FreeSlot.TimeText->SetText(FText::AsNumber(RemainInt));
            FreeSlot.TimeText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }

        IdToSlotIndex.Add(StatusEffectId, FreeIndex);
    }
}

void UPC_StatusEffectWidget::RemoveStatusEffect(uint32 StatusEffectId)
{
	if (int32* FoundIndex = IdToSlotIndex.Find(StatusEffectId))
	{
		const int32 Index = *FoundIndex;
		if (Slots.IsValidIndex(Index))
		{
			FStatusEffectSlot& StatusEffectSlot = Slots[Index];
			if (StatusEffectSlot.Icon)
			{
				StatusEffectSlot.Icon->SetVisibility(ESlateVisibility::Collapsed);
				FSlateBrush Empty;
				StatusEffectSlot.Icon->SetBrush(Empty);
			}

			if (StatusEffectSlot.TimeText)
			{
				StatusEffectSlot.TimeText->SetVisibility(ESlateVisibility::Collapsed);
				StatusEffectSlot.TimeText->SetText(FText::GetEmpty());
			}
		}

		IdToSlotIndex.Remove(StatusEffectId);
	}
}

void UPC_StatusEffectWidget::GenerateFillOrder()
{
	FillOrder.Reset();

	if (MaxIcons <= 0)
		return;

	if (MaxIcons % 2 == 1)
	{
		// 홀수 개 (예: 5 → [2,1,3,0,4])
		const int32 Center = MaxIcons / 2;
		FillOrder.Add(Center);

		for (int32 Offset = 1; Offset <= Center; ++Offset)
		{
			int32 Left = Center - Offset;
			int32 Right = Center + Offset;
			if (Left >= 0)
				FillOrder.Add(Left);
			if (Right < MaxIcons)
				FillOrder.Add(Right);
		}
	}
	else
	{
		// 짝수 개 (예: 6 → [2,3,1,4,0,5])
		const int32 RightCenter = MaxIcons / 2;
		const int32 LeftCenter = RightCenter - 1;

		FillOrder.Add(LeftCenter);
		FillOrder.Add(RightCenter);

		for (int32 Offset = 1; ; ++Offset)
		{
			int32 Left = LeftCenter - Offset;
			int32 Right = RightCenter + Offset;
			if (Left < 0 && Right >= MaxIcons)
				break;
			if (Left >= 0)
				FillOrder.Add(Left);
			if (Right < MaxIcons)
				FillOrder.Add(Right);
		}
	}
}

