#include "PC_DamageFloaterWidget.h"

#include <string>

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PC/PC.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

UPC_DamageFloaterWidget::UPC_DamageFloaterWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UPC_DamageFloaterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ElapsedTime = 0.f;
}

void UPC_DamageFloaterWidget::Init(int32 InDamge, const FVector& InWorldLocation, APlayerController* InPlayerController)
{
	FString DamageString = FString::FromInt(InDamge);

	if (Text_Damage)
		Text_Damage->SetText(FText::FromString(DamageString));

	SpawnWorldLocation = InWorldLocation + FVector(0.f, 0.f, SpawnOffsetZ);
	OwnerController = InPlayerController;

	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	SetRenderScale(FVector2D(SpawnScale, SpawnScale));

	if (OwnerController.IsValid())
	{
		FVector2D ScreenPos;
		if (OwnerController->ProjectWorldLocationToScreen(SpawnWorldLocation, ScreenPos, true))
		{
			const float OffX = (ScreenOffsetX > 0.f) ? FMath::FRandRange(-ScreenOffsetX, ScreenOffsetX) : 0.f;
			const float OffY = (ScreenOffsetY > 0.f) ? FMath::FRandRange(-ScreenOffsetY, ScreenOffsetY) : 0.f;
			SpawnScreedOffset = FVector2d(OffX, OffY);

			FVector2D SpawnPos = ScreenPos + SpawnScreedOffset;

			int32 VX, VY;
			OwnerController->GetViewportSize(VX, VY);
			SpawnPos.X = FMath::Clamp(SpawnPos.X, ScreenPadding, static_cast<float>(VX) - ScreenPadding);
			SpawnPos.Y = FMath::Clamp(SpawnPos.Y, ScreenPadding, static_cast<float>(VY) - ScreenPadding);

			SmoothedScreenPos = SpawnPos;
			SetPositionInViewport(SmoothedScreenPos, true);
			bHandValidProjection = true;
		}
	}
}

void UPC_DamageFloaterWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

	if (ScaleInDuration > 0.f)
	{
		const float RawAlpha = FMath::Clamp(ElapsedTime / ScaleInDuration, 0.f, 1.f);
		const float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, RawAlpha, ScaleEaseExponent);
		const float ScaleNow = FMath::Lerp(SpawnScale, TargetScale, EasedAlpha);
		SetRenderScale(FVector2D(ScaleNow, ScaleNow));
	}
	else
	{
		SetRenderScale(FVector2D(TargetScale, TargetScale));
	}

	const float RiseElapsed = FMath::Max(0.f, ElapsedTime - RiseDelay);

	if (OwnerController.IsValid())
	{
		const FVector WorldPos = SpawnWorldLocation + FVector(0, 0, RiseSpeed * RiseElapsed);

		FVector2D Screen;
		const bool bProjected = OwnerController->ProjectWorldLocationToScreen(WorldPos, Screen, true);
		if (bProjected)
		{
			FVector2D Desired = Screen + SpawnScreedOffset;

			int32 VX, VY;
			OwnerController->GetViewportSize(VX, VY);
			Desired.X = FMath::Clamp(Desired.X, ScreenPadding, static_cast<float>(VX) - ScreenPadding);
			Desired.Y = FMath::Clamp(Desired.Y, ScreenPadding, static_cast<float>(VY) - ScreenPadding);

			if(ScreedLerpSpeed > 0.f && bHandValidProjection)
			{
				SmoothedScreenPos = FMath::Vector2DInterpTo(SmoothedScreenPos, Desired, InDeltaTime,ScreenPadding);
				SetPositionInViewport(SmoothedScreenPos, true);
			}
			else
			{
				SmoothedScreenPos = Desired;
				SetPositionInViewport(Desired, true);
			}

			if(!bHandValidProjection)
			{
				SetVisibility(ESlateVisibility::Visible);
				bHandValidProjection = true;
			}
		}
		else
		{
			SetVisibility(ESlateVisibility::Hidden);
			bHandValidProjection = false;
		}
	}

	float alpha = 1.f;
	if(FadeDuration > 0.f)
	{
		const float fadeStart = FMath::Max(0.f, LifeTime - FadeDuration);
		if(ElapsedTime >= fadeStart)
		{
			//const float t = FMath::Clamp(ElapsedTime - fadeStart / FadeDuration, 0.f, 1.f);
			
			const float t = FMath::Clamp((ElapsedTime - fadeStart) / FadeDuration, 0.f, 1.f);
			alpha = 1.f - t;
			UE_LOG(LogPC, Log, TEXT("Alpha: %f"), alpha);
		}
	}
	SetRenderOpacity(alpha);

	if(ElapsedTime >= LifeTime)
	{
		RemoveFromParent();
	}
}
