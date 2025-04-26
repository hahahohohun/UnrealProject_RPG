// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_PatrolRoute.h"

// Sets default values
APC_PatrolRoute::APC_PatrolRoute()
{
	PatrolSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("splineComponent"));
	SetRootComponent(PatrolSplineComponent);
}

// Called when the game starts or when spawned
void APC_PatrolRoute::BeginPlay()
{
	Super::BeginPlay();
}

void APC_PatrolRoute::IncrementIndex()
{
	check(PatrolSplineComponent);
	const int SplinePoint = PatrolSplineComponent->GetNumberOfSplinePoints();

	CurrentPatrolIndex = CurrentPatrolIndex + Direction;

	if (CurrentPatrolIndex == SplinePoint - 1)
	{
		Direction = -1;
	}
	else if (CurrentPatrolIndex == 0)
	{
		Direction = 1;
	}
}

FVector APC_PatrolRoute::GetLocationAtSplinePoint()
{
	check(PatrolSplineComponent);
	return PatrolSplineComponent->GetLocationAtSplinePoint(CurrentPatrolIndex, ESplineCoordinateSpace::World);
}
