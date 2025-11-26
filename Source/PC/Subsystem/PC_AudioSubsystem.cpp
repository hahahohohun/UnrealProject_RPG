// PC_AudioSubsystem.cpp

#include "PC_AudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

void UPC_AudioSubsystem::PlayBGM(USoundBase* BGMSound, float FadeTime)
{
	if (!BGMSound)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 기존 BGM 정리
	if (IsValid(BGMAudioComponent))
	{
		BGMAudioComponent->FadeOut(FadeTime, 0.f);
		BGMAudioComponent = nullptr; 
	}

	// 새로운 BGM 생성
	BGMAudioComponent = UGameplayStatics::SpawnSound2D(World, BGMSound, BGMVolume);
	if (BGMAudioComponent)
	{
		BGMAudioComponent->bIsUISound = false;
		BGMAudioComponent->bAllowSpatialization = false;
		BGMAudioComponent->bAutoDestroy = false;

		BGMAudioComponent->FadeIn(FadeTime, BGMVolume);
	}
}

void UPC_AudioSubsystem::StopBGM(float FadeTime)
{
	if (BGMAudioComponent)
	{
		BGMAudioComponent->FadeOut(FadeTime, 0.f);
		BGMAudioComponent = nullptr;
	}
}

void UPC_AudioSubsystem::PlaySFXAtLocation(USoundBase* SFX, const FVector& Location)
{
	if (!SFX)
		return;

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(World, SFX, Location, SFXVolume);
	}
}

void UPC_AudioSubsystem::PlaySFX2D(USoundBase* SFX)
{
	if (!SFX)
		return;

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::PlaySound2D(World, SFX, SFXVolume);
	}
}

void UPC_AudioSubsystem::SetBGMVolume(float Volume)
{
	BGMVolume = Volume;

	if (BGMAudioComponent)
	{
		BGMAudioComponent->FadeIn(1.f ,BGMVolume);
	}
}

void UPC_AudioSubsystem::SetSFXVolume(float Volume)
{
	SFXVolume = Volume;
}
