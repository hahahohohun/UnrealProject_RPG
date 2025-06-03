// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BattleComponent.h"

#include "MovieSceneFwd.h"
#include "PC_StatComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "PC/PC.h"
#include "PC/Character/PC_BaseCharacter.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

// Sets default values for this component's properties
UPC_BattleComponent::UPC_BattleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	TraceInterval = 0.01f;
}


void UPC_BattleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bTracing)
		return;

	TraceElapsedTime += DeltaTime;
	if (TraceElapsedTime < TraceInterval)
		return;

	TraceElapsedTime = 0.f;

	APC_BaseCharacter* ClassCharacter = Cast<APC_BaseCharacter>(GetOwner());
	if (!ClassCharacter)
		return;

	const USkeletalMeshComponent* Mesh = ClassCharacter->GetMesh();
	if (!Mesh)
		return;

	const UWorld* World = GetWorld();
	if (!World)
		return;

	FVector CurStartBoneLocation = FVector::ZeroVector;
	FVector CurEndBoneLocation = FVector::ZeroVector;

	if (HasWeapon())
	{
		if (const IPC_PlayerCharacterInterface* Interface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
		{
			UStaticMeshComponent* WeaponMesh = Interface->GetWeaponStaticMeshComponent();
			check(WeaponMesh);

			CurStartBoneLocation = WeaponMesh->GetSocketLocation(TraceStartBoneName);
			CurEndBoneLocation = WeaponMesh->GetSocketLocation(TraceEndBoneName);
		}
	}
	else
	{
		CurStartBoneLocation = Mesh->GetBoneLocation(TraceStartBoneName);
		CurEndBoneLocation = Mesh->GetBoneLocation(TraceEndBoneName);
	}

	// Trace 할 라인들 모음
	TArray<TPair<FVector, FVector>> TraceLines;
	TraceLines.Emplace(PrevStartBoneLocation, CurStartBoneLocation);  
	TraceLines.Emplace(PrevEndBoneLocation, CurEndBoneLocation);
	TraceLines.Emplace(PrevStartBoneLocation, CurEndBoneLocation);    
	TraceLines.Emplace(PrevEndBoneLocation, CurStartBoneLocation);    
	TraceLines.Emplace(CurStartBoneLocation, CurEndBoneLocation);

	int32 SegmentCount = 3;
	
	// 💡 추가: 분할 점 기반 연결선
	for (int32 i = 1; i < SegmentCount; ++i)
	{
		const float Alpha = static_cast<float>(i) / SegmentCount;

		const FVector PrevMid = FMath::Lerp(PrevStartBoneLocation, PrevEndBoneLocation, Alpha);
		const FVector CurrMid = FMath::Lerp(CurStartBoneLocation, CurEndBoneLocation, Alpha);

		TraceLines.Emplace(PrevMid, CurrMid);
	}
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	for (const auto& Line : TraceLines)
	{
		FHitResult HitResult;
		ECollisionChannel CollisionChannel = ClassCharacter->CharacterType == EPC_CharacterType::Player?
			ECC_GameTraceChannel2 : ECC_GameTraceChannel1;
		
		if (World->LineTraceSingleByChannel(HitResult, Line.Key, Line.Value, CollisionChannel, Params))
		{
			AActor* HitActor = HitResult.GetActor();

			if (HitActor && !DamagedActor.Contains(HitActor))
			{
				if (APC_BaseCharacter* HitCharacter = Cast<APC_BaseCharacter>(HitActor))
				{
					UE_LOG(LogPC, Log, TEXT("Hit!!"));

					DamagedActor.Add(HitActor);

					const float Damage = ClassCharacter->StatComponent->GetTotalStat().Attack;

					FDamageEvent DamageEvent;
					HitActor->TakeDamage(Damage, DamageEvent, ClassCharacter->GetController(), ClassCharacter);

					SpawnEffect(HitResult.ImpactPoint);
				}
			}
		}
		
		DrawDebugLine(World, Line.Key, Line.Value, FColor::Red, false, 3.f, 0, 1.f);
	}

	// Prev 갱신
	PrevStartBoneLocation = CurStartBoneLocation;
	PrevEndBoneLocation = CurEndBoneLocation;
}

void UPC_BattleComponent::StartTraceWithWeapon()
{
	bTracing = true;
	
	TraceStartBoneName = CurrentWeaponTableRow->TraceStartSocketName;
	TraceEndBoneName = CurrentWeaponTableRow->TraceEndSocketName;

	if (const IPC_PlayerCharacterInterface* Interface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
	{
		UStaticMeshComponent* WeaponStaticMeshComponent = Interface->GetWeaponStaticMeshComponent();
		check(WeaponStaticMeshComponent);

		PrevStartBoneLocation = WeaponStaticMeshComponent->GetSocketLocation(TraceStartBoneName);
		PrevEndBoneLocation = WeaponStaticMeshComponent->GetSocketLocation(TraceEndBoneName);
	}
}

void UPC_BattleComponent::StartTrace(FName InTraceStartBoneName, FName InTraceEndBoneName)
{
	bTracing = true;
	
	TraceStartBoneName = InTraceStartBoneName;
	TraceEndBoneName = InTraceEndBoneName;
	
	const ACharacter* Character = CastChecked<ACharacter>(GetOwner());
	const USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh();
	check(SkeletalMeshComponent);
	
	PrevStartBoneLocation = SkeletalMeshComponent->GetSocketLocation(TraceStartBoneName);
	PrevEndBoneLocation = SkeletalMeshComponent->GetSocketLocation(TraceEndBoneName);
}

void UPC_BattleComponent::EquipWeapon(uint8 InWeaponId)
{
	CurrentWeaponTableRow = FPC_GameUtil::GetWeaponData(InWeaponId);
	if (!CurrentWeaponTableRow)
	{
		UnEquipWeapon();
		return;
	}
	
	FName WeaponSocketName = NAME_None;
	
	if (const IPC_PlayerCharacterInterface* Interface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
	{
		UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
		check(PlayerData);

		WeaponSocketName = PlayerData->WeaponSocketName;

		const ACharacter* Character = CastChecked<ACharacter>(GetOwner());
		USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh();
		check(SkeletalMeshComponent);
		
		UStaticMeshComponent* WeaponStaticMeshComponent = Interface->GetWeaponStaticMeshComponent();
		check(WeaponStaticMeshComponent);

		WeaponStaticMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		WeaponStaticMeshComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
		
		WeaponStaticMeshComponent->SetRelativeLocation(CurrentWeaponTableRow->RelativePos);
		WeaponStaticMeshComponent->SetRelativeRotation(CurrentWeaponTableRow->RelativeRot);
		
		WeaponStaticMeshComponent->SetStaticMesh(CurrentWeaponTableRow->WeaponMesh);
		WeaponStaticMeshComponent->SetVisibility(true);
	}
}

void UPC_BattleComponent::UnEquipWeapon()
{
	CurrentWeaponTableRow = nullptr;

	if (const IPC_PlayerCharacterInterface* Interface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
	{
		UStaticMeshComponent* WeaponStaticMeshComponent = Interface->GetWeaponStaticMeshComponent();
		check(WeaponStaticMeshComponent);

		WeaponStaticMeshComponent->SetStaticMesh(nullptr);
		WeaponStaticMeshComponent->SetVisibility(false);
	}
}

bool UPC_BattleComponent::HasWeapon()
{
	if (CurrentWeaponTableRow)
		return true;

	return false;
}

void UPC_BattleComponent::EndTrace()
{
	DamagedActor.Empty();
	bTracing = false;
	TraceElapsedTime = 0.f;
}

void UPC_BattleComponent::SpawnEffect(FVector InHitLocation)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	UParticleSystem* LoadedParticle = Cast<UParticleSystem>(StaticLoadObject(UParticleSystem::StaticClass(), nullptr,
		TEXT("/Game/ParagonCrunch/FX/Particles/Abilities/Hook/FX/P_Crunch_Hook_Impact.P_Crunch_Hook_Impact")));

	if (LoadedParticle)
		UGameplayStatics::SpawnEmitterAtLocation(World, LoadedParticle, InHitLocation, FRotator::ZeroRotator);
	
}
