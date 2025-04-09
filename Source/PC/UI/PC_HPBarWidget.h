// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "PC_HPBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_HPBarWidget : public UPC_UserWidget
{
	GENERATED_BODY()
public:
	UPC_HPBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	
public:
	void UpdateHpBar(float NewCurrentHp, float MaxHp);
	FString GetHPText();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<class UProgressBar> PB_HP;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TB_HP;

	UPROPERTY()
	float CurrentHp;

	UPROPERTY()
	float MaxHp;
};
