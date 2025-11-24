// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PC/PC_Enum.h"
#include "PCGameMode.generated.h"

UCLASS(minimalapi)
class APCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APCGameMode();
	virtual void BeginPlay() override;

	bool DebugDrawing = false;
	APostProcessVolume* GetCombatPPVolume() const { return CombatPPVolume; }
	UMaterialInstanceDynamic* GetCombatPPMID() const { return CombatPPMID; }
	void PlayBGM(EPC_BGMType BGMType);
	
protected:
	UPROPERTY()
	APostProcessVolume* CombatPPVolume = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* CombatPPMID = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* StageBGMSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* BattleBGMSound = nullptr;
};



