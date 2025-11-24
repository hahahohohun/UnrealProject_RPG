// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "PC/UI/PC_UserWidget.h"
#include "PC/UI/PC_DamageFloaterWidget.h"
#include "PC/UI/PC_HUDWidget.h"
#include "PC/UI/PC_OptionSettingWidget.h"        // 위젯 클래스
#include "PC/UI/PC_StatusEffectWidget.h"
#include "PC_UISubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_UISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPC_UISubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	template<typename T>
	UPC_UserWidget* CreateUI(T* ClassType);

	void CreateHUD();
	
	UPC_OptionSettingWidget* ToggleOptionsWidget();
	//
	
	UPC_DamageFloaterWidget* CreateDamageFloater(AActor* Owner);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPC_HUDWidget> HPBarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPC_StatusEffectWidget> StatusEffectWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPC_OptionSettingWidget> OptionSettingWidgetClass;

	void HideBossHPWidget();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<UPC_HUDWidget> HUDWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPC_DamageFloaterWidget> DamageFloaterWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HUD)
	TObjectPtr<UPC_HUDWidget> HUDWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPC_OptionSettingWidget> OptionSettingWidget;

private:
	UPC_OptionSettingWidget* CreateOptionSettingWidget();
	
};

template <typename T>
UPC_UserWidget* UPC_UISubsystem::CreateUI(T* ClassType)
{
	if (!IsValid(ClassType))
		return nullptr;

	if (UPC_UserWidget* CreatedWidget = Cast<UPC_UserWidget>(CreateWidget(GetWorld(), ClassType)))
	{
		return CreatedWidget;
	}
	
	return nullptr;
}
