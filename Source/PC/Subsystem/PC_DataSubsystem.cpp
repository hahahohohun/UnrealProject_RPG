// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_DataSubsystem.h"

#include "Engine/DataTable.h"

UPC_DataSubsystem::UPC_DataSubsystem()
{
}

void UPC_DataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	TArray<TPair<EPC_DataTableType, FSoftObjectPath>> LoadList;
	LoadList.Add(TPair<EPC_DataTableType, FSoftObjectPath>(EPC_DataTableType::CharacterStat, CharacterStat_DataTablePath));
	LoadList.Add(TPair<EPC_DataTableType, FSoftObjectPath>(EPC_DataTableType::Enemy, EnemyStat_DataTablePath));
	LoadList.Add(TPair<EPC_DataTableType, FSoftObjectPath>(EPC_DataTableType::Weapon, Weapon_DataTablePath));

	for (const auto& Element : LoadList)
	{
		const FSoftObjectPath DataTableAssetPath(Element.Value);
		if(DataTableAssetPath.IsValid())
		{
			if(UDataTable* Table = Cast<UDataTable>(DataTableAssetPath.TryLoad()))
			{
				DataTableMap.Add(Element.Key, Table);
			}
		}
	}

	const FSoftObjectPath NormalCameraAssetObject(NormalCameraDataPath);
	if (NormalCameraAssetObject.IsValid())
	{
		if (UPC_CameraDataAsset* DataAsset = Cast<UPC_CameraDataAsset>(NormalCameraAssetObject.TryLoad()))
		{
			CameraData.Add(EPC_CameraType::Normal, DataAsset);
		}
	}

	const FSoftObjectPath AimCameraAssetObject(AimCameraDataPath);
	if (AimCameraAssetObject.IsValid())
	{
		if (UPC_CameraDataAsset* DataAsset = Cast<UPC_CameraDataAsset>(AimCameraAssetObject.TryLoad()))
		{
			CameraData.Add(EPC_CameraType::Aim, DataAsset);
		}
	}
}

UDataTable* UPC_DataSubsystem::GetTable(EPC_DataTableType TableType)
{
	if(DataTableMap.Contains(TableType))
	{
		return DataTableMap[TableType];
	}
	return nullptr;
}
