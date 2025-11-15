#pragma once
#include "CoreMinimal.h"
#include "PC_OptionData.generated.h"

USTRUCT(BlueprintType)
struct FPC_OptionData
{
	GENERATED_BODY()

	// 그래픽
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFoliage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseShadow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAA = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsePostProcess = true;

	// 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
	float BGMVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
	float SFXVolume = 1.0f;

	// 카메라 감도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1", ClampMax="5.0"))
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1", ClampMax="5.0"))
	float GamepadSensitivity = 1.0f;

	// 키 바인딩 (간단 버전)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FKey> KeyBindings;
};
