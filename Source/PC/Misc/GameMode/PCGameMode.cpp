// Copyright Epic Games, Inc. All Rights Reserved.

#include "PCGameMode.h"

#include "EngineUtils.h"
#include "PC/Subsystem/PC_AudioSubsystem.h"
#include "UObject/ConstructorHelpers.h"

APCGameMode::APCGameMode()
{
	
}

void APCGameMode::BeginPlay()
{
	Super::BeginPlay();

	//필드에 배치된게 한개라 우선 이렇게 
	for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
	{
		if (It->bUnbound)
		{
			CombatPPVolume = *It;
			break;
		}
	}

	if (!CombatPPVolume)
		return;

	if (CombatPPVolume->Settings.WeightedBlendables.Array.Num() > 0)
	{
		FWeightedBlendable& Blendable = CombatPPVolume->Settings.WeightedBlendables.Array[0];

		if (UMaterialInterface* BaseMat = Cast<UMaterialInterface>(Blendable.Object))
		{
			CombatPPMID = UMaterialInstanceDynamic::Create(BaseMat, this);

			Blendable.Object = CombatPPMID;
			Blendable.Weight = 1.f; 
		}
	}

	PlayBGM(EPC_BGMType::Stage);
}

void APCGameMode::PlayBGM(EPC_BGMType BGMType)
{
	if(BGMType == EPC_BGMType::None)
		return;
	
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UPC_AudioSubsystem* Audio = GI->GetSubsystem<UPC_AudioSubsystem>())
		{
			if(BGMType == EPC_BGMType::Stage)
				Audio->PlayBGM(StageBGMSound);
			else if(BGMType == EPC_BGMType::Combat)
				Audio->PlayBGM(BattleBGMSound);
		}
	}
}
