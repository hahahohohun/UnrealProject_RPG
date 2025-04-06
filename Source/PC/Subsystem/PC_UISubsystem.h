// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC/UI/PC_UserWidget.h"
#include "PC/UI/PC_HUDWidget.h"
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
	
	template<typename T>
	UPC_UserWidget* CreateUI(T* ClassType);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<UPC_HUDWidget> HUDWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HUD)
	TObjectPtr<UPC_HUDWidget> HUDWidget;
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
