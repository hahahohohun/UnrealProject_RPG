// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC/Data//PC_TableRows.h"
#include "PC_BattleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_BattleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPC_BattleComponent();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartTraceWithWeapon();
	void StartTrace(FName InTraceStartBoneName, FName InTraceEndBoneName);
	void EndTrace();
	
	void SpawnEffect(FVector InHitLocation);

	void EquipWeapon(uint8 InWeaponId);
	void UnEquipWeapon();
	bool HasWeapon();
	
	bool bTracing = false;

	FName TraceStartBoneName;
	FName TraceEndBoneName;

	FVector PrevStartBoneLocation = FVector::ZeroVector;
	FVector PrevEndBoneLocation = FVector::ZeroVector;

	float TraceInterval = 0.f;
	float TraceElapsedTime = 0.f;
	
	TArray<TWeakObjectPtr<AActor>> DamagedActor;
	
	FPC_WeaponTableRow* CurrentWeaponTableRow = nullptr;
};


