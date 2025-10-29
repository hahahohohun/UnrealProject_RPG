// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "PC/Character/Component/PC_StatusEffectComponent.h"
#include "PC_StatusEffectWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_StatusEffectWidget : public UPC_UserWidget
{
	GENERATED_BODY()

public:
	UPC_StatusEffectWidget(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	void UpdateStatusEffect( uint32 StatusEffectId, float RemainingTime);
	UFUNCTION()
	void AddOrUpdateStatusEffect(uint32 StatusEffectId);
	UFUNCTION()
	void RemoveStatusEffect(uint32 StatusEffectId);

private:
	void GenerateFillOrder();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<class UHorizontalBox> StatusContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
	int32 MaxIcons = 5;

	UPROPERTY()
	TMap<uint32, TObjectPtr<class UImage>> IdToImage;

	UPROPERTY()
	TArray<TObjectPtr<class UImage>> Slots; //이미지 보관

private:
	TArray<uint32> FillOrder;
};
