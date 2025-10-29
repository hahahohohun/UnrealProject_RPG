// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "PC_DamageFloaterWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_DamageFloaterWidget : public UPC_UserWidget
{
	GENERATED_BODY()

public:
	UPC_DamageFloaterWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void Init(int32 InDamge, const FVector& InWorldLocation, APlayerController* InPlayerController);

protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<class UTextBlock> Text_Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float LifeTime = 1.0f;

	//올라가는 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float RiseSpeed = 50.f;

	//사라지는 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floater")
	float FadeDuration = 0.35f;

	//위치 보간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float ScreedLerpSpeed = 12.f;

	//생성 시 scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float SpawnScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float TargetScale = 0.08f;

	//목표 scale까지 도달하는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float ScaleInDuration = 0.08f;

	//상승 움직임 전까지 딜레이 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float RiseDelay = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float ScaleEaseExponent = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float ScreenOffsetX = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float ScreenOffsetY = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float SpawnOffsetZ = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floater")
	float ScreenPadding = 8.f;

private:
	FVector2D SpawnScreedOffset = FVector2D::ZeroVector;

	TWeakObjectPtr<APlayerController> OwnerController;
	FVector SpawnWorldLocation = FVector::ZeroVector;
	FVector2D SmoothedScreenPos = FVector2D::ZeroVector;

	float ElapsedTime = 0.0f;
	bool bHandValidProjection = false;
};
