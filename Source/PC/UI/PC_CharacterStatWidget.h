// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_UserWidget.h"
#include "PC/Data/PC_TableRows.h"
#include "PC_CharacterStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_CharacterStatWidget : public UPC_UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
public:
	void UpdateStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat);

private:
	UPROPERTY()
	TMap<FName, class UTextBlock*> BaseLookup;

	UPROPERTY()
	TMap<FName, class UTextBlock*> ModifierLookup;
};
