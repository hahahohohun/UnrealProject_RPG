// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "PC_PatrolRoute.generated.h"

UCLASS()
class PC_API APC_PatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APC_PatrolRoute();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	void IncrementIndex();
	FVector GetLocationAtSplinePoint();

protected:
	int CurrentPatrolIndex;
	int Direction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USplineComponent> PatrolSplineComponent = nullptr;
};
