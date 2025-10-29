// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC/UI/PC_DamageFloaterWidget.h"
#include "PC/UI/PC_UserWidget.h"
#include "PC/UI/PC_HUDWidget.h"
#include "PC/UI/PC_StatusEffectWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
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

	void CreateHUD();

	UPC_DamageFloaterWidget* CreateDamageFloater(AActor* Owner);
	
	template<typename T>
	UPC_UserWidget* CreateUI(T* ClassType);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPC_HUDWidget> HPBarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPC_StatusEffectWidget> StatusEffectWidgetClass;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<UPC_HUDWidget> HUDWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HUD)
	TObjectPtr<UPC_HUDWidget> HUDWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPC_DamageFloaterWidget> DamageFloaterWidgetClass;
	
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
