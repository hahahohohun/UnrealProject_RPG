// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PC/PC_Enum.h"
#include "PC/Data/PC_CameraDataAsset.h"
#include "Subsystems/Subsystem.h"
#include "PC_DataSubsystem.generated.h"

/**
 * 
 */
class UPC_CameraDataAsset;
class UDataTable;

UCLASS(config=GAME)
class PC_API UPC_DataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPC_DataSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UDataTable* GetTable(EPC_DataTableType TableType);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EPC_DataTableType, UDataTable*> DataTableMap;

	TPair<EPC_DataTableType, UDataTable*> DataTableMaps;

	UPROPERTY()
	TMap<EPC_CameraType, UPC_CameraDataAsset*> CameraData;
	
private:
	UPROPERTY(Config)
	FSoftObjectPath CharacterStat_DataTablePath;

	UPROPERTY(Config)
	FSoftObjectPath EnemyStat_DataTablePath;
	
	UPROPERTY(config)
	FSoftObjectPath Weapon_DataTablePath;

	UPROPERTY(Config)
	FSoftObjectPath NormalCameraDataPath;
	
	UPROPERTY(config)
	FSoftObjectPath AimCameraDataPath;
};
