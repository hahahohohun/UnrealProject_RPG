// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_SeesawOscillator.h"

// Sets default values
APC_SeesawOscillator::APC_SeesawOscillator()
{
	PrimaryActorTick.bCanEverTick = true;

	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	SetRootComponent(Pivot);

	Platform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	Platform->SetupAttachment(Pivot);
	Platform->SetMobility(EComponentMobility::Movable);
	Platform->SetSimulatePhysics(false);               // 물리와 싸우지 않게
	Platform->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

}

// Called when the game starts or when spawned
void APC_SeesawOscillator::BeginPlay()
{
	Super::BeginPlay();
	InitialRelRot = Platform->GetRelativeRotation();
}

void APC_SeesawOscillator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Super::Tick(DeltaTime);
	TimeAcc += DeltaTime;

	const float PhaseRad = FMath::DegreesToRadians(PhaseDeg);
	const float Angle = AmplitudeDeg * FMath::Sin(2.f * PI * FrequencyHz * TimeAcc + PhaseRad);

	FRotator Offset(0,0,0);
	switch (Axis)
	{
	case EPC_OscAxis::Roll_X:  Offset.Roll  = Angle; break;   // X
	case EPC_OscAxis::Pitch_Y: Offset.Pitch = Angle; break;   // Y (시소)
	case EPC_OscAxis::Yaw_Z:   Offset.Yaw   = Angle; break;   // Z
	}

	Platform->SetRelativeRotation(InitialRelRot + Offset);
}

