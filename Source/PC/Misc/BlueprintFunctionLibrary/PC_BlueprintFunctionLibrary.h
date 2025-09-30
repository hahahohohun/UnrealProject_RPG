// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PC_BlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PC_API UPC_BlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static void CollectAssetPaths(const TArray<FString>& Paths, bool bRecursive, UClass* AssetClass, TArray<FSoftObjectPath>& OutPaths);
	
	UFUNCTION(BlueprintCallable)
	static bool FillDataTableWithRMDistances(UDataTable* DataTable, const TArray<FSoftObjectPath>& MontagePaths, bool bClear);

	UFUNCTION(BlueprintCallable)
	static bool FillDataTableWithHitPartBones(UDataTable* DataTable, UDataAsset* HitPartData, const TArray<FSoftObjectPath>& PhysicsAssetPaths, bool bClear);
	
private:
	static void NormalizeAndScan(const TArray<FString>& InPaths, TArray<FName>& OutPackagePaths);
};
