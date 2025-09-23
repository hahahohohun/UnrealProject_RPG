// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "PC_BossHPBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BossHPBarWidget : public UPC_UserWidget
{
	GENERATED_BODY()
public:
	UPC_BossHPBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	
public:
	void UpdateHpBar(float NewCurrentHp, float NewMaxHp);
	FString GetHPText();
	void Init(ACharacter* InTargetCharacter);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<class UProgressBar> PB_HP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TB_HP;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_Name;
	
	UPROPERTY()
	float CurrentHp;

	UPROPERTY()
	float MaxHp;
};
