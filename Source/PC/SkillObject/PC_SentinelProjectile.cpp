// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_SentinelProjectile.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "PC/Character/PC_BaseCharacter.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "Engine/EngineTypes.h"
#include "PC/Utills/PC_GameUtill.h"

APC_SentinelProjectile::APC_SentinelProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 감지용 스피어 추가(사거리 진입 감지)
	DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectSphere"));
	DetectSphere->SetupAttachment(RootComponent); // Root는 APC_SkillObject의 Collision_Environment

	// ProjectileMovementComponent는 APC_SkillObject에서 이미 생성됨
	ProjectileMovementComponent->bAutoActivate = false; // 오비트 중 비활성
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
}

void APC_SentinelProjectile::InitSentinel(const FPC_SentinelParams& InParams)
{
	Cfg = InParams;
	ThetaDeg = Cfg.StartAngleDeg;
	IdleTimeLeft = Cfg.IdleLifeTime;

	AActor* owner = GetOwner();
	check(owner);

	APC_BaseCharacter* Character = Cast<APC_BaseCharacter>(GetOwner());
	if (!Character)
		return;

	//Trace Enemy, Trace Player
	ECollisionChannel CollisionChannel = FPC_GameUtil::GetAttackCollisionChannel(Character->CharacterDataID);
	// 감지/근접 반경 적용
	if (DetectSphere)
	{
		DetectSphere->SetSphereRadius(Cfg.TriggerRange);
		//DetectSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		//DetectSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		//Enemy Preset, Player Preset
		DetectSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
	}

	// 근접 판정은 APC_SkillObject의 TriggerCollision을 재사용(Overlap 전용)
	if (TriggerCollision)
	{
		TriggerCollision->SetSphereRadius(Cfg.DamageRadius);
		//TriggerCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		//TriggerCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
		//TriggerCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	// 환경 충돌(Blocking/Hit) 사용 안 함 → 완전 비활성
	if (Collision_Environment)
	{
		Collision_Environment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Collision_Environment->SetSphereRadius(3.f);
	}

	// Overlap 바인딩
	if (DetectSphere)
	{
		DetectSphere->Activate(false);
		
		if (!DetectSphere->OnComponentBeginOverlap.IsAlreadyBound(this, &ThisClass::OnDetectBegin))
			DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnDetectBegin);
		if (!DetectSphere->OnComponentEndOverlap.IsAlreadyBound(this, &ThisClass::OnDetectEnd))
			DetectSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnDetectEnd);
	}
	
	if (TriggerCollision && !TriggerCollision->OnComponentBeginOverlap.IsAlreadyBound(
		this, &ThisClass::OnProximityBegin))
		TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProximityBegin);

	// 수명은 오비트 동안은 수동관리(IdleTimeLeft), 발사 후에는 PostLaunchLife를 LifeTime으로 설정
	LifeTime = TNumericLimits<float>::Max(); // 오비트 모드에선 Tick에서 IdleTimeLeft로 종료
	DelayAttackTime = Cfg.DelayAttackTime;
	ElapsedTime = 0.f; // APC_SkillObject 틱 누적 초기화

	Phase = EPC_SentinelPhase::Orbit;
}

void APC_SentinelProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 혹시 APC_SkillObject 쪽 BeginPlay에서 Projectile용 바인딩/프로필을 설정했을 경우를 대비해
	// Sentinel은 블로킹 충돌을 쓰지 않으므로 Hit 델리게이트/프로필이 묶였었다면 풀어두는 걸 권장
	if (Collision_Environment)
	{
		Collision_Environment->OnComponentHit.RemoveAll(this);
	}
}

void APC_SentinelProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Phase == EPC_SentinelPhase::Orbit)
	{
		UpdateOrbit(DeltaSeconds);

		DelayAttackTime -= DeltaSeconds;
		if (DetectSphere)
		{
			if (DetectSphere->IsActive() == false)
			{
				DetectSphere->Activate(DelayAttackTime <= 0.f);
			}
		}

		IdleTimeLeft -= DeltaSeconds;
		if (IdleTimeLeft <= 0.f)
		{
			FinishSelf();
			return;
		}

		if (Candidates.Num() > 0)
		{
			ExecutePickAndLaunch();
		}
	}
	// Launched: ProjectileMovement가 이동 담당, 여기서는 추가 갱신 불필요
}

void APC_SentinelProjectile::UpdateOrbit(float deltaTime)
{
	if (!OwnerCharacter.IsValid())
	{
		FinishSelf();
		return;
	}

	ThetaDeg = FMath::Fmod(ThetaDeg + Cfg.AngularSpeed * deltaTime, 360.f);
	const float Rad = FMath::DegreesToRadians(ThetaDeg);

	const FVector Center = OwnerCharacter->GetActorLocation() + FVector(0, 0, Cfg.OrbitHeight);
	const FVector Offset(FMath::Cos(Rad) * Cfg.OrbitRadius, FMath::Sin(Rad) * Cfg.OrbitRadius, 0.f);
	const FVector NewLoc = Center + Offset;

	SetActorLocation(NewLoc, false);
	SetActorRotation((Center - NewLoc).Rotation());
}

bool APC_SentinelProjectile::IsEnemyValid(AActor* Other) const
{
	if (!Other || Other == GetOwner())
		return false;

	// 같은 진영 제외(플레이어가 쏜 건 몹만, 몹이 쏜 건 플레이어만)
	if (OwnerCharacter.IsValid())
	{
		const bool bOwnerIsPlayer = OwnerCharacter->Implements<UPC_PlayerCharacterInterface>();
		const bool bOtherIsPlayer = Other->Implements<UPC_PlayerCharacterInterface>();
		if (bOwnerIsPlayer == bOtherIsPlayer)
			return false;
	}

	if (auto* CharacterInterface = Cast<IPC_CharacterInterface>(Other))
	{
		if (CharacterInterface->IsDead())
			return false;
	}

	return true;
}

void APC_SentinelProjectile::OnDetectBegin(UPrimitiveComponent*, AActor* OtherActor,
                                           UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (IsEnemyValid(OtherActor))
	{
		Candidates.Add(OtherActor);
	}
}

void APC_SentinelProjectile::OnDetectEnd(UPrimitiveComponent*, AActor* OtherActor,
                                         UPrimitiveComponent*, int32)
{
	Candidates.Remove(OtherActor);

	if (Target.Get() == OtherActor)
		Target = nullptr;
}

AActor* APC_SentinelProjectile::FindBestTarget() const
{
	if (!OwnerCharacter.IsValid()) return nullptr;

	AActor* Best = nullptr;
	float BestDis = TNumericLimits<float>::Max();
	const FVector From = GetActorLocation();

	for (auto& W : Candidates)
	{
		if (AActor* A = W.Get())
		{
			const float Dis = FVector::DistSquared(From, A->GetActorLocation());
			if (Dis < BestDis)
			{
				Best = A;
				BestDis = Dis;
			}
		}
	}

	return Best;
}

void APC_SentinelProjectile::ExecutePickAndLaunch()
{
	if (Phase != EPC_SentinelPhase::Orbit) return;

	AActor* Picked = FindBestTarget();
	if (!Picked) return;

	EnterLaunch(Picked);
}

void APC_SentinelProjectile::EnterLaunch(AActor* InTarget)
{
	Phase = EPC_SentinelPhase::Launched;
	Target = InTarget;

	// 발사 후 생존시간은 APC_SkillObject의 LifeTime/ElapsedTime으로 관리
	LifeTime = 1;
	ElapsedTime = 0.f;

	// Projectile 이동 시작(호밍/직진)
	ProjectileMovementComponent->Activate(true);
	ProjectileMovementComponent->InitialSpeed = Cfg.InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = Cfg.MaxSpeed;

	if (Cfg.bHoming)
	{
		if (USceneComponent* TargetComp = InTarget->GetRootComponent())
		{
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = Cfg.HomingAccel;
			ProjectileMovementComponent->HomingTargetComponent = TargetComp;

			const FVector Dir = (TargetComp->GetComponentLocation() - GetActorLocation()).GetSafeNormal();
			ProjectileMovementComponent->Velocity = Dir * Cfg.InitialSpeed;
		}
	}
	else
	{
		const FVector Dir = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		ProjectileMovementComponent->Velocity = Dir * Cfg.InitialSpeed;
	}
}

void APC_SentinelProjectile::OnProximityBegin(UPrimitiveComponent*, AActor* OtherActor,
                                              UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	// 발사 후 근접 오버랩 시 데미지 1회
	if (Phase != EPC_SentinelPhase::Launched) return;
	if (!IsEnemyValid(OtherActor)) return;

	// 타깃이 잠겨있다면 그 타깃에게만
	if (Target.IsValid() && OtherActor != Target.Get()) return;

	// 데미지는 SkillObject 테이블 값 사용
	if (FPC_SkillObjectTableRow* Row = FPC_GameUtil::GetSkillObjectData(SkillObjectId))
	{
		FDamageEvent DamageEvent;
		OtherActor->TakeDamage(Row->Damage,
		                       DamageEvent,
		                       OwnerCharacter.IsValid() ? OwnerCharacter->GetController() : nullptr,
		                       OwnerCharacter.Get());
	}

	FinishSelf();
}

void APC_SentinelProjectile::FinishSelf()
{
	if (Phase == EPC_SentinelPhase::Finished) return;
	Phase = EPC_SentinelPhase::Finished;

	// FX/사운드 및 파괴는 APC_SkillObject의 ProcessDestroy 이용
	ProcessDestroy();
}
