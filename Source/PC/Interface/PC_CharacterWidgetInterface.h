// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC/UI/PC_LockOnWidget.h"
#include "UObject/Interface.h"
#include "PC_CharacterWidgetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPC_CharacterWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PC_API IPC_CharacterWidgetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//
	virtual void SetupCharacterWidget(class UPC_UserWidget* InWidget) = 0;
	virtual void SetupLockOnWidget(class UPC_UserWidget* InUserWidget) = 0;

	virtual void OnLocked(bool bLocked) = 0;
};
