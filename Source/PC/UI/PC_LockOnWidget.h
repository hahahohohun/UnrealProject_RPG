// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "Components/Image.h"
#include "PC_LockOnWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_LockOnWidget : public UPC_UserWidget
{
	GENERATED_BODY()
public:
	UPC_LockOnWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	void ToggleActivation(bool bEnable);

protected:
	virtual void NativeConstruct() override;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_LockOn2; //이미지 이름 맞춰야함
};
