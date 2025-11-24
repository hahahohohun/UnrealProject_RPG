// Fill out your copyright notice in the Description page of Project Settings.


#include "PC/Subsystem/PC_OptionSubsystem.h"

#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "PC/Character/PC_PlayableCharaceter.h"
#include "PC/OptionSetting/PC_OptionConfigDataAsset.h"
#include "PC/OptionSetting/PC_OptionSaveGame.h"

class APC_PlayableCharaceter;

UPC_OptionSubsystem::UPC_OptionSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UPC_OptionConfigDataAsset> ConfigAssetObj(
		TEXT("DataAsset'/Game/ProjectClass/Data/DataAsset/Option/DA_Option_Default.DA_Option_Default'"));

	if (ConfigAssetObj.Succeeded())
	{
		ConfigAsset = ConfigAssetObj.Object;
		UE_LOG(LogTemp, Log, TEXT("Option DataAsset 생성자 로드 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Option DataAsset 생성자 로드 실패! 경로: %s"), *ConfigAssetObj.GetReferencerName());
	}
}

void UPC_OptionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	LoadOption();
	ApplyGraphicsOptions();
	ApplyAudioOptions();
}

void UPC_OptionSubsystem::LoadOption()
{
	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex))
	{
		if (UPC_OptionSaveGame* OptionSave = Cast<UPC_OptionSaveGame>(Loaded))
		{
			CurrentOption = OptionSave->SavedOption;
			return;
		}
	}

	// SaveGame 없으면 DataAsset 기본값 사용
	if (ConfigAsset)
	{
		CurrentOption = ConfigAsset->DefaultOption;
	}
}

void UPC_OptionSubsystem::SaveOption()
{
	UPC_OptionSaveGame* SaveObj = Cast<UPC_OptionSaveGame>(
	UGameplayStatics::CreateSaveGameObject(UPC_OptionSaveGame::StaticClass()));

	SaveObj->SavedOption = CurrentOption;
	UGameplayStatics::SaveGameToSlot(SaveObj, SaveSlotName, SaveUserIndex);

	if(ConfigAsset)
	{
		ConfigAsset->SaveOption = CurrentOption;
	}
}

void UPC_OptionSubsystem::RestOption()
{
	UPC_OptionSaveGame* SaveObj = Cast<UPC_OptionSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UPC_OptionSaveGame::StaticClass()));

	if (ConfigAsset)
	{
		SaveObj->SavedOption = ConfigAsset->DefaultOption;
		CurrentOption = ConfigAsset->DefaultOption;
	}

	UGameplayStatics::SaveGameToSlot(SaveObj, SaveSlotName, SaveUserIndex);
}

void UPC_OptionSubsystem::ApplyAndSaveOption(const FPC_OptionData& NewOption)
{
	CurrentOption = NewOption;

	ApplyGraphicsOptions();
	ApplyAudioOptions();
	
	SaveOption();
}

void UPC_OptionSubsystem::ApplyGraphicsOptions()
{
	if (!GEngine)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (World)
	{
		Scalability::FQualityLevels Quality = UserSettings->ScalabilityQuality;
		
		Quality.FoliageQuality = CurrentOption.bUseFoliage ? 2 : 0;
		
		UKismetSystemLibrary::ExecuteConsoleCommand(
			World,
			FString::Printf(TEXT("sg.FoliageQuality %d"), CurrentOption.bUseFoliage ? 2 : 0)
		);
		
		// 섀도우
		Quality.ShadowQuality = CurrentOption.bUseShadow ? 2 : 0;

		// 안티 에일리어싱
		Quality.AntiAliasingQuality = CurrentOption.bUseAA ? 2 : 0;

		// 포스트 프로세싱
		Quality.PostProcessQuality = CurrentOption.bUsePostProcess ? 2 : 0;

		// 바뀐 값 반영
		UserSettings->ScalabilityQuality = Quality;
		UserSettings->ApplySettings(false); // 저장은 SaveOption 따로
	}
}

void UPC_OptionSubsystem::ApplyAudioOptions()
{
}

//카메라
void UPC_OptionSubsystem::ApplyCameraOptions(APlayerController* PC)
{
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (APC_PlayableCharaceter* MyChar = Cast<APC_PlayableCharaceter>(Pawn))
	{
		const float Sens =
			FMath::GetMappedRangeValueClamped(
				FVector2D(0.f, 100.f),      // 슬라이더
				FVector2D(0.1f, 2.0f),      // 실제 감도
				(float)CurrentOption.MouseSensitivity);

		MyChar->SetMouseSensitivity(Sens);
	}
}

//키 입력
void UPC_OptionSubsystem::ApplyKeyBindings(class ULocalPlayer* LocalPlayer)
{
	
}
