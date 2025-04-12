// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BattleComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "PC/Character/PC_BaseCharacter.h"

// Sets default values for this component's properties
UPC_BattleComponent::UPC_BattleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPC_BattleComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	TraceInterval = 0.01f;
}


// Called every frame
void UPC_BattleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if(IsTracing)
	{
		TraceElapsedTime += DeltaTime;
		if(TraceElapsedTime > TraceInterval)
		{
			TraceElapsedTime = 0;

			APC_BaseCharacter* Character = Cast<APC_BaseCharacter>(GetOwner());
			if(!Character)
				return;

			AController* Controller = Character->GetController();
			if (!Controller)
				return;

			USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh();
			if(!SkeletalMeshComponent)
				return;

			UWorld* World = GetWorld();
			if(!World)
				return;

			const FVector CurrentLocation = SkeletalMeshComponent->GetSocketLocation(TraceBoneName);
			const float Height = (PrevLocation - CurrentLocation).Length();

			FRotator Rot = (PrevLocation - CurrentLocation).Rotation();

			const FRotationMatrix Original(Rot);
			const FVector RightVector = Original.GetUnitAxis(EAxis::Y);
			const FQuat OffsetQuat = FQuat(RightVector, FMath::DegreesToRadians(90));

			const FQuat NewQuat = OffsetQuat * Rot.Quaternion();

			TArray<FHitResult> HitResults;

			World->SweepMultiByChannel(HitResults, PrevLocation, CurrentLocation, NewQuat, ECC_GameTraceChannel1,
	FCollisionShape::MakeCapsule(20.f, Height * 0.5f), FCollisionQueryParams::DefaultQueryParam);

			DrawDebugCapsule(World, FMath::Lerp(CurrentLocation, PrevLocation, 0.5f), Height * 0.5f, 20.f, NewQuat,
		FColor::Red, false, 3.f);

			if (HitResults.Num() != 0 )
			{
				for (FHitResult& Result : HitResults)
				{
					if (!DamageActors.Contains(Result.GetActor()))
					{
						UE_LOG(LogTemp, Log, TEXT("Hit!!"));

						UGameplayStatics::ApplyDamage(Result.GetActor(), 10.f, Controller,
							Character, UDamageType::StaticClass());
						
						DamageActors.Add(Result.GetActor());
						SpawnEffect(Result.ImpactPoint);
					}
				}
			}

			PrevLocation = CurrentLocation;
		}
	}
}
void UPC_BattleComponent::StartTrace(FName InTraceBoneName)
{
	TraceBoneName = InTraceBoneName;
	IsTracing = true;

	if(APC_BaseCharacter* Character = Cast<APC_BaseCharacter>(GetOwner()))
	{
		if(USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh())
		{
			//시작할때 본 위치 캐싱
			PrevLocation = SkeletalMeshComponent->GetSocketLocation(InTraceBoneName);
		}
	}
}

void UPC_BattleComponent::EndTrace()
{
	//초기화
	DamageActors.Empty();
	IsTracing = false;
	TraceElapsedTime = 0.f;
}

void UPC_BattleComponent::SpawnEffect(FVector InHitLocation)
{
	//"/Game/ParagonCrunch/FX/Particles/Abilities/Hook/FX/P_Crunch_Hook_Impact.P_Crunch_Hook_Impact"
UWorld* World = GetWorld();
	if(!World)
		return;

	UParticleSystem* LoadParticle = Cast<UParticleSystem>(StaticLoadObject(UParticleSystem::StaticClass(),nullptr,
		TEXT("/Game/ParagonCrunch/FX/Particles/Abilities/Hook/FX/P_Crunch_Hook_Impact.P_Crunch_Hook_Impact")));

	if(LoadParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, LoadParticle, InHitLocation, FRotator::ZeroRotator);
	}
}

