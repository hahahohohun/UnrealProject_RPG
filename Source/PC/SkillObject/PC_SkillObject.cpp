// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_SkillObject.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "PC/Battle/PC_NormalAttackDamageType.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

class IPC_CharacterAIInterface;
// Sets default values
APC_SkillObject::APC_SkillObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision_Environment = CreateDefaultSubobject<USphereComponent>(TEXT("Collision_Environment"));
	RootComponent = Collision_Environment;

	TriggerCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	TriggerCollision->SetupAttachment(RootComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
}

// Called when the game starts or when spawned
void APC_SkillObject::BeginPlay()
{
	Super::BeginPlay();

	FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(SkillObjectId);
	check(SkillObjectTableRow);

	if (SkillObjectTableRow->SkillObjectType == EPC_SkillObjectType::Projectile)
	{
		if (!TriggerCollision->OnComponentBeginOverlap.IsAlreadyBound(this, &ThisClass::OnBeginOverlap))
			TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);

		check(Collision_Environment);
		Collision_Environment->SetCollisionProfileName(TEXT("SkillObject_Environment"));
		Collision_Environment->SetSphereRadius(1.f);

		if (!Collision_Environment->OnComponentHit.IsAlreadyBound(this, &ThisClass::OnComponentHit))
			Collision_Environment->OnComponentHit.AddDynamic(this, &ThisClass::OnComponentHit);

		if (BounceCount > 0)
			ProjectileMovementComponent->bShouldBounce = true;

		if (ShowImpactPointDecal)
		{
			PlayImpactPointDecal();
		}
	}
}

// Called every frame
void APC_SkillObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;
	if (ElapsedTime > LifeTime)
	{
		ProcessDestroy();
	}

	if(FPC_GameUtil::IsDebugDrawing(this))
	{
		if (Collision_Environment)
		{
			const FVector EnvLoc = Collision_Environment->GetComponentLocation();
			const float EnvRadius = Collision_Environment->GetScaledSphereRadius();
			DrawDebugSphere(GetWorld(), EnvLoc, EnvRadius, 16, FColor::Green, false, -1.f, 0, 2.f);
		}

		if (TriggerCollision)
		{
			const FVector TriggerLoc = TriggerCollision->GetComponentLocation();
			const float TriggerRadius = TriggerCollision->GetScaledSphereRadius();
			DrawDebugSphere(GetWorld(), TriggerLoc, TriggerRadius, 16, FColor::Red, false, -1.f, 0, 2.f);
		}
	}
}

void APC_SkillObject::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                     const FHitResult& SweepResult)
{
	FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(SkillObjectId);
	check(SkillObjectTableRow);
	
	if (ACharacter* HitCharacter = Cast<ACharacter>(OtherActor))
	{
		FPointDamageEvent DamageEvent;

		bool bHitPartUnit = false;
		if(IPC_CharacterAIInterface* CharacterAIInterface = Cast<IPC_CharacterAIInterface>(HitCharacter))
		{
			if(auto EnemyData = CharacterAIInterface->GetEnemyData())
				bHitPartUnit = EnemyData->IsHitPartUnit;
		}

		float AddDamage = 0.0f;
		if(bHitPartUnit)
		{
			USkeletalMeshComponent* MeshComp = HitCharacter->GetMesh();
			if (MeshComp)
			{
				UWorld* World = GetWorld();
				if (!World)
					return;

				FHitResult MeshHit;
				FCollisionObjectQueryParams ObjectQueryParams;
				ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);
				Params.AddIgnoredActor(OwnerCharacter.Get());
				Params.bTraceComplex = true;

				const FVector Start = GetActorLocation();
				const FVector End   = HitCharacter->GetActorLocation();
				
				if (World->LineTraceSingleByObjectType(MeshHit, Start, End, ObjectQueryParams, Params))
				{
					DamageEvent.DamageTypeClass = UPC_NormalAttackDamageType::StaticClass();
					DamageEvent.HitInfo = MeshHit;
					if(IPC_CharacterAIInterface* CharacterAIInterface = Cast<IPC_CharacterAIInterface>(HitCharacter))
					{
						if (auto HitPartList = CharacterAIInterface->GetHitPartList())
						{
							AddDamage += FPC_GameUtil::GetHitPartAddDamage(HitPartList, MeshHit.BoneName);
						}
					}
				}
			}
		}
		else
		{
			DamageEvent.DamageTypeClass = UPC_NormalAttackDamageType::StaticClass();
			DamageEvent.HitInfo = SweepResult;
		}

		const float FinalDamage = SkillObjectTableRow->Damage + AddDamage;
		HitCharacter->TakeDamage(
			FinalDamage,
			DamageEvent,
			OwnerCharacter->GetController(),
			OwnerCharacter.Get());
	}
	
	if(OnDelegateBeginOverlap.IsBound())
		OnDelegateBeginOverlap.Broadcast(OtherActor);
	
	if(SkillObjectTableRow->IsCollisionDestroy)
	{
		ProcessDestroy();
	}
}

void APC_SkillObject::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void APC_SkillObject::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (BounceCount > 0)
	{
		BounceCount--;
	}
	else if (BounceCount == 0)
	{
		ProcessDestroy();
	}
}

void APC_SkillObject::PlaySound()
{
	FVector ActorLocation = GetActorLocation();
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DeSpawnSound, ActorLocation);
}

void APC_SkillObject::PlayFX(FVector InHitLocation)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DespawnFX, GetActorLocation(), GetActorRotation());
	//FPC_GameUtil::CameraShake(EPC_CameraShakeMagnitudeType::Weak);
}

void APC_SkillObject::PlayImpactPointDecal()
{
	if (!ProjectileMovementComponent)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector StartLoc = GetActorLocation();

	// 1. 실제 발사 속도를 사용
	FVector LaunchVel = ProjectileMovementComponent->Velocity;

	FPredictProjectilePathParams Params;
	Params.StartLocation       = StartLoc;
	Params.LaunchVelocity      = LaunchVel;
	Params.bTraceWithCollision = true;
	Params.SimFrequency        = 15.f; // 더 촘촘히
	Params.TraceChannel        = ECC_Visibility;
	Params.ProjectileRadius    = Collision_Environment
								 ? Collision_Environment->GetScaledSphereRadius()
								 : 0.f;

	// 2. 중력 맞춰주기
	const float WorldGravityZ      = World->GetGravityZ();
	const float ProjectileGravityZ = WorldGravityZ * ProjectileMovementComponent->ProjectileGravityScale;
	Params.OverrideGravityZ = ProjectileGravityZ;

	// 3. 수명에 맞게 시뮬레이션 시간 확장
	Params.MaxSimTime = LifeTime > 0.f ? LifeTime : 5.f;

	FPredictProjectilePathResult Result;
	if (UGameplayStatics::PredictProjectilePath(this, Params, Result))
	{
		const FHitResult& Hit = Result.HitResult;

		FVector DecalSize(60.f);
		FRotator DecalRotation = FVector::UpVector.Rotation();

		UGameplayStatics::SpawnDecalAtLocation(
			GetWorld(),
			ImpactPointDecalMaterial,
			DecalSize,
			Hit.ImpactPoint,
			DecalRotation,
			2.0f);
	}
}

void APC_SkillObject::ProcessDestroy()
{
	TriggerCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collision_Environment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovementComponent->Deactivate();
	StaticMeshComponent->SetVisibility(false);
	OnDelegateBeginOverlap.Clear();
	ActiveCrowdId = -1;
	PlaySound();
	PlayFX(GetActorLocation());

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		Destroy();
	}), 0.5f, false);
}

void APC_SkillObject::SetCrowdControl(FPC_OnBeginOverlap OnBeginOverlap)
{
	OnDelegateBeginOverlap.Clear();
	OnDelegateBeginOverlap = OnBeginOverlap;
}
