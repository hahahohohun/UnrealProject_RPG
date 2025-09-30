// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_BlueprintFunctionLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Utills/PC_GameUtill.h"
#include "PhysicsEngine/PhysicsAsset.h"

void UPC_BlueprintFunctionLibrary::CollectAssetPaths(const TArray<FString>& Paths, bool bRecursive, UClass* AssetClass,
	TArray<FSoftObjectPath>& OutPaths)
{
	OutPaths.Reset();
	if (!AssetClass || Paths.Num() == 0) return;

	TArray<FName> PackagePaths;
	NormalizeAndScan(Paths, PackagePaths);

	FARFilter Filter;
	Filter.bRecursivePaths = bRecursive;
	Filter.bRecursiveClasses = true;
	Filter.ClassPaths.Add(AssetClass->GetClassPathName());
	
	for (const FName& P : PackagePaths)
		Filter.PackagePaths.Add(P);

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssets(Filter, Assets);

	TSet<FSoftObjectPath> Unique;
	Unique.Reserve(Assets.Num());
	OutPaths.Reserve(Assets.Num());
	
	for (const FAssetData& AD : Assets)
	{
		OutPaths.AddUnique(AD.ToSoftObjectPath());
	}
}

bool UPC_BlueprintFunctionLibrary::FillDataTableWithRMDistances(UDataTable* DataTable,
	const TArray<FSoftObjectPath>& MontagePaths, bool bClear)
{
	if (!DataTable || DataTable->GetRowStruct() != FPC_AnimMontageRootMotionDistanceRow::StaticStruct())
		return false;

	if (bClear)
		DataTable->EmptyTable();

	// 기존 Path → RowNames 매핑(업데이트용)
	TMap<FString, FName> PathToRowName;
	
	const TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (const FPC_AnimMontageRootMotionDistanceRow* Row = DataTable->FindRow<FPC_AnimMontageRootMotionDistanceRow>(RowName, TEXT(""), false))
		{
			const FString Key = Row->MontagePath.ToString();
			PathToRowName.Add(Key, RowName);
		}
	}
	
	for (const FSoftObjectPath& SoftPath : MontagePaths)
	{
		FString PathStr = SoftPath.ToString();
		TSoftObjectPtr<UAnimMontage> SoftObjectPtr(SoftPath);

		UAnimMontage* Montage = SoftObjectPtr.LoadSynchronous();
		const float Distance = FPC_GameUtil::CalculateRootMotionDistance(Montage);

		if (const FName* Name = PathToRowName.Find(PathStr))
		{
			if (FPC_AnimMontageRootMotionDistanceRow* Existing = DataTable->FindRow<FPC_AnimMontageRootMotionDistanceRow>(*Name, TEXT(""), false))
			{
				Existing->MontagePath = SoftPath;
				Existing->Distance = Distance;
			}
			
			continue;
		}

		// 신규 추가
		FPC_AnimMontageRootMotionDistanceRow NewRow;
		NewRow.MontagePath = SoftPath;
		NewRow.Distance = Distance;

		FName BaseName = SoftPath.GetAssetFName();
		const FName UniqueRowName = MakeUniqueObjectName(DataTable, UDataTable::StaticClass(), BaseName);
		
		DataTable->AddRow(UniqueRowName, NewRow);
		PathToRowName.Add(PathStr, UniqueRowName);
	}

	DataTable->MarkPackageDirty();
	
	return true;
}

bool UPC_BlueprintFunctionLibrary::FillDataTableWithHitPartBones(UDataTable* DataTable, UDataAsset* HitPartData,
	const TArray<FSoftObjectPath>& PhysicsAssetPaths, bool bClear)
{
	if (!DataTable || DataTable->GetRowStruct() != FPC_HitPartListRow::StaticStruct())
		return false;
	
	if (bClear)
		DataTable->EmptyTable();

	TMap<FString, FName> PathToRowName;
	{
		TArray<FName> RowNames = DataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			if (FPC_HitPartListRow* Row = DataTable->FindRow<FPC_HitPartListRow>(RowName, TEXT(""), false))
			{
				const FString Key = Row->PhysicsAssetPath.ToString();
				PathToRowName.Add(Key, RowName);
			}
		}
	}

	for (const FSoftObjectPath& SoftObjectPath : PhysicsAssetPaths)
	{
		FString PathStr = SoftObjectPath.ToString();
		TSoftObjectPtr<UPhysicsAsset> SoftObjectPtr(SoftObjectPath);
		
		UPhysicsAsset* PhysicsAsset = SoftObjectPtr.LoadSynchronous();

		TArray<FPC_HitPartData> HitPartDatas;
		for (USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
		{
			if (BodySetup)
			{
				FPC_HitPartData Data;
				Data.HitPartName = BodySetup->BoneName;
				Data.HitPartType = FPC_GameUtil::GetHitPartTypeByName(BodySetup->BoneName, HitPartData);
				HitPartDatas.Add(Data);
			}
		}
		
		if (const FName* RowName = PathToRowName.Find(PathStr))
		{
			if (FPC_HitPartListRow* Existing = DataTable->FindRow<FPC_HitPartListRow>(*RowName, TEXT(""), false))
			{
				Existing->PhysicsAssetPath = SoftObjectPath;
				Existing->HitPartDatas = HitPartDatas;
			}

			continue;
		}

		FPC_HitPartListRow NewRow;
		NewRow.PhysicsAssetPath = SoftObjectPath;
		NewRow.HitPartDatas = HitPartDatas;

		const FName Base = SoftObjectPath.GetAssetFName();
		const FName RowName = MakeUniqueObjectName(DataTable, UDataTable::StaticClass(), Base);

		DataTable->AddRow(RowName, NewRow);
		PathToRowName.Add(PathStr, RowName);
	}

	DataTable->MarkPackageDirty();
	return true;
}

void UPC_BlueprintFunctionLibrary::NormalizeAndScan(const TArray<FString>& InPaths, TArray<FName>& OutPackagePaths)
{
	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AR = ARM.Get();

	TArray<FString> Normalized;
	Normalized.Reserve(InPaths.Num());

	for (const FString& P : InPaths)
	{
		FString S = P.Replace(TEXT("/All/"), TEXT("/"));
		if (!S.StartsWith(TEXT("/Game/")))
			S = FString(TEXT("/Game/")) + S.TrimStartAndEnd();
		if (!S.EndsWith(TEXT("/")))
			S += TEXT("/");

		Normalized.Add(S);
	}

	AR.ScanPathsSynchronous({ Normalized }, true);

	OutPackagePaths.Reset(Normalized.Num());
	for (const FString& P : Normalized)
	{
		if (!P.IsEmpty())
			OutPackagePaths.Add(FName(*P));
	}
}
