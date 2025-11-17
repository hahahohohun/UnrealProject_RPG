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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="100"))
	int32 BGMVolume = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="100"))
	int32 SFXVolume = 100;

	// 카메라 감도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="100"))
	int32 MouseSensitivity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="5"))
	int32 GamepadSensitivity = 1.0f;

	// 키 바인딩 (간단 버전)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FKey> KeyBindings;

public:
	void Rest()
	{
		bUseFoliage = true;
		bUseShadow = true;
		bUseAA = true;
		bUsePostProcess = true;
		BGMVolume = 100;
		SFXVolume = 100;
		MouseSensitivity = 50.0f;
		GamepadSensitivity = 1.0f;
	}
};
