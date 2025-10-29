#include "PC_StatusEffectWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/PanelSlot.h"
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
	for(int32 i = 0; i < MaxIcons; i++)
	{
		UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		
		if(UHorizontalBoxSlot* ImgSlot = StatusContainer->AddChildToHorizontalBox(Img))
		{
			ImgSlot->SetPadding(FMargin(2.f,0.f));
			ImgSlot->SetHorizontalAlignment(HAlign_Center);
			ImgSlot->SetVerticalAlignment(VAlign_Center);
		}
		Img->SetVisibility(ESlateVisibility::Collapsed);
		Slots.Add(Img);
	}
	
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPC_StatusEffectWidget::UpdateStatusEffect( uint32 StatusEffectId, float RemainingTime)
{
	if(RemainingTime > 0.f)
		AddOrUpdateStatusEffect(StatusEffectId);
	else
		RemoveStatusEffect(StatusEffectId);
}

void UPC_StatusEffectWidget::AddOrUpdateStatusEffect(uint32 StatusEffectId)
{
	if (IsValid(OwningActor))
	{
		if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwningActor))
		{
			UPC_StatusEffectComponent* StatusEffectComponent = CharacterInterface->GetStatusEffectComponent();
			if (StatusEffectComponent)
			{
				FPC_StatusEffectInfo* EffectInfo = StatusEffectComponent->GetActiveStatusEffectInfo(StatusEffectId);
				if (EffectInfo != nullptr)
				{
					FPC_StatusEffectTableRow* statusEffectTableRow = FPC_GameUtil::GetStatusEffectData(StatusEffectId);
					check(statusEffectTableRow);

					if (auto Found = IdToImage.Find(EffectInfo->StatusEffectId))
					{
						if (UImage* Img = *Found)
						{
							FSlateBrush Brush;
							Brush.SetResourceObject(statusEffectTableRow->Icon);
							Brush.ImageSize = FVector2D(45.f, 45.f);
							Img->SetBrush(Brush);
							Img->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
						}
						return;
					}

					// 빈 슬롯 찾기
					UImage* FreeSlot = nullptr;
					for (UImage* Img : Slots)
					{
						if (Img->GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
						{
							FreeSlot = Img;
							break;
						}
					}

					//UImage* FreeSlot = nullptr;
					//for (int32 idx : FillOrder)
					//{
					//	UImage* Img = Slots.IsValidIndex(idx) ? Slots[idx] : nullptr;
					//	if (Img && Img->GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
					//	{
					//		FreeSlot = Img;
					//		break;
					//	}
					//}

					if (!FreeSlot)
						return;

					FSlateBrush Brush;
					Brush.SetResourceObject(statusEffectTableRow->Icon);
					Brush.ImageSize = FVector2D(45.f, 45.f);
					FreeSlot->SetBrush(Brush);
					FreeSlot->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

					IdToImage.Add(EffectInfo->StatusEffectId, FreeSlot);
				}
			}
		}
	}
}

void UPC_StatusEffectWidget::RemoveStatusEffect(uint32 StatusEffectId)
{
	if (auto Found = IdToImage.Find(StatusEffectId))
	{
		if (UImage* Img = *Found)
		{
			Img->SetVisibility(ESlateVisibility::Collapsed);
			FSlateBrush Empty;
			Img->SetBrush(Empty);
		}
		IdToImage.Remove(StatusEffectId);
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

