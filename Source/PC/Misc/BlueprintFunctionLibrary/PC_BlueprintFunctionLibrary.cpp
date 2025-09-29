// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_BlueprintFunctionLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Utills/PC_GameUtill.h"
#include "PhysicsEngine/PhysicsAsset.h"

TArray<TSoftObjectPtr<UAnimMontage>> UPC_BlueprintFunctionLibrary::CollectAnimMontagesByPaths(
	const TArray<FString>& Paths, bool bRecursive)
{
	TArray<TSoftObjectPtr<UAnimMontage>> Result;

	if(Paths.Num() == 0)
	{
		return Result;
	}

	Collect_Internal(Paths, bRecursive, Result);
	return Result;
}

void UPC_BlueprintFunctionLibrary::Collect_Internal(const TArray<FString>& PackagePaths, bool bRecursive,
	TArray<TSoftObjectPtr<UAnimMontage>>& OutMontages)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AR = AssetRegistryModule.Get();

	TArray<FString> NormalizedPaths;
	for(const FString& Path : PackagePaths)
	{
		FString NormalizedPath = Path.Replace(TEXT("/All/"), TEXT("/"));

		if(!NormalizedPath.StartsWith(TEXT("/Game/")))
		{
			NormalizedPath = FString(TEXT("/Game/")) + NormalizedPath.TrimStartAndEnd();
		}

		if(!NormalizedPath.EndsWith(TEXT("/")))
		{
			NormalizedPath += TEXT("/");
		}

		NormalizedPaths.Add(NormalizedPath);
		
	}

	AR.ScanPathsSynchronous({ NormalizedPaths }, true);

	TArray<FName> PackageNamePaths;
	PackageNamePaths.Reserve(NormalizedPaths.Num());
	for (const FString& P : NormalizedPaths)
	{
		if (!P.IsEmpty())
			PackageNamePaths.Add(FName(*P));
	}

	FARFilter Filter;
	Filter.ClassPaths.Add(UAnimMontage::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = bRecursive;
	Filter.bRecursiveClasses = true;
	for (const FName& P : PackageNamePaths)
	{
		Filter.PackagePaths.Add(P);
	}
	
	TArray<FAssetData> Assets;
	AR.GetAssets(Filter, Assets);

	OutMontages.Reserve(Assets.Num());

	for (const FAssetData& AD : Assets)
	{
		const FSoftObjectPath SoftPath = AD.ToSoftObjectPath();
		OutMontages.Add(TSoftObjectPtr<UAnimMontage>(SoftPath));
	}
}

bool UPC_BlueprintFunctionLibrary::FillDataTableWithRmDistances(UDataTable* DataTable,
	const TArray<TSoftObjectPtr<UAnimMontage>>& Montages, bool bClear)
{
	if(!DataTable || DataTable->GetRowStruct() != FPC_AnimMontageRootMotionDistanceRow::StaticStruct())
	{
		return false;
	}

	if(bClear)
	{
		DataTable->EmptyTable();
	}

	TMap<FString, TArray<FName>> PathToRowNames;
	TArray<FName> RowNames = DataTable->GetRowNames();

	for(const FName& RN : RowNames)
	{
		if (FPC_AnimMontageRootMotionDistanceRow* Row =
			DataTable->FindRow<FPC_AnimMontageRootMotionDistanceRow>(RN, TEXT("FillRMDistances"), false))
		{
			PathToRowNames.FindOrAdd(Row->MontagePath.ToString()).Add(RN);
		}
	}

	for(const TSoftObjectPtr<UAnimMontage>& softmontage : Montages)
	{
		UAnimMontage* Montage = softmontage.LoadSynchronous();

		FString PathStr = Montage->GetPathName();
		FSoftObjectPath SoftObjectPath(PathStr);

		const float Distance = FPC_GameUtil::CalculateRootMotionDistance_Internal(Montage);

		if(TArray<FName>* FoundRows = PathToRowNames.Find(PathStr))
		{
			for(const FName& RN : *FoundRows)
			{
				if(FPC_AnimMontageRootMotionDistanceRow* Existing =
					DataTable->FindRow<FPC_AnimMontageRootMotionDistanceRow>(RN,TEXT(""),false))
				{
					Existing->MontagePath = SoftObjectPath;
					Existing->Distance = Distance;
				}
			}

			continue;
		}

		FPC_AnimMontageRootMotionDistanceRow NewRow;
		NewRow.MontagePath = SoftObjectPath;
		NewRow.Distance = Distance;

		FName BaseName = SoftObjectPath.GetAssetFName();
		FName UniqueRowName = MakeUniqueObjectName(DataTable, UDataTable::StaticClass(), BaseName);

		DataTable->AddRow(UniqueRowName, NewRow);
		PathToRowNames.FindOrAdd(PathStr).Add(UniqueRowName);
	}

	DataTable->MarkPackageDirty();

	return true;
}