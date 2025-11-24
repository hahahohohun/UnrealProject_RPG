// PC_AudioSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PC_AudioSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class PC_API UPC_AudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void PlayBGM(USoundBase* BGMSound, float FadeTime = 0.5f);

	UFUNCTION(BlueprintCallable)
	void StopBGM(float FadeTime = 0.5f);

	UFUNCTION(BlueprintCallable)
	void PlaySFXAtLocation(USoundBase* SFX, const FVector& Location);

	UFUNCTION(BlueprintCallable)
	void PlaySFX2D(USoundBase* SFX);

	void SetBGMVolume(float Volume);
	void SetSFXVolume(float Volume);

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMAudioComponent = nullptr;

	float BGMVolume = 1.0f;
	float SFXVolume = 1.0f;
};
