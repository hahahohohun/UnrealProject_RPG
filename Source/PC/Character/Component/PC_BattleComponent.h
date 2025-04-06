// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC_BattleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_BattleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPC_BattleComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void StartTrace(FName TraceBoneName);

	UFUNCTION()
	void EndTrace();

	UFUNCTION()
	void SpawnEffect(FVector InHitLocation);

	bool IsTracing = false;
	FName TraceBoneName;
	
	FVector PrevLocation = FVector::ZeroVector;

	float TraceInterval = 0.0f;
	float TraceElapsedTime = 0.0f;

	TArray<TWeakObjectPtr<AActor>> DamageActors;
};


