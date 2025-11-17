// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC/OptionSetting/PC_OptionData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PC_OptionSubsystem.generated.h"


class UPC_OptionConfigDataAsset;
/**
 * 
 */
UCLASS()
class PC_API UPC_OptionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPC_OptionSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	const FPC_OptionData& GetCurrentOption() const { return CurrentOption; }

	// 전체 옵션 변경 & 적용 & 저장
	void ApplyAndSaveOption(const FPC_OptionData& NewOption);
	
	void ApplyGraphicsOptions();
	void ApplyAudioOptions();
	
	void ApplyCameraOptions(APlayerController* PC);
	void ApplyKeyBindings(class ULocalPlayer* LocalPlayer);
	void RestOption();

private:
	void LoadOption();
	void SaveOption();

private:
	FPC_OptionData CurrentOption;

	UPROPERTY()
	UPC_OptionConfigDataAsset* ConfigAsset = nullptr;

	FString SaveSlotName = TEXT("PC_OptionSlot");
	uint32 SaveUserIndex = 0;
};