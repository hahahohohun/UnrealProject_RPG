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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TArray<TSoftObjectPtr<UAnimMontage>> CollectAnimMontagesByPaths(const TArray<FString>& Paths, bool bRecursive = true);

	static void Collect_Internal(const TArray<FString>& PackagePaths, bool bRecursive, TArray<TSoftObjectPtr<UAnimMontage>>& OutMontages);

	UFUNCTION(BlueprintCallable)
	static bool FillDataTableWithRmDistances(UDataTable* DataTable, const TArray<TSoftObjectPtr<UAnimMontage>>& Montages, bool bClear = true);
};
