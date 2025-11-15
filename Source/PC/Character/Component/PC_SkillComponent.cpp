#include "PC_SkillComponent.h"

//#include <tiffio.h>

//#include "SAdvancedRotationInputBox.h"
#include "PC_CrowdControlComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
//#include "Framework/Text/ShapedTextCache.h"
//#include "Camera/CameraComponent.h"
//#include "GameFramework/SpringArmComponent.h"
//#include "Kismet/KismetMathLibrary.h"
//#include "PC/Data/PC_CameraDataAsset.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PC/Character/PC_BaseCharacter.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/SkillObject/PC_SentinelProjectile.h"
#include "PC/SkillObject/PC_SkillObject.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_SkillComponent::UPC_SkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UPC_SkillComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UPC_SkillComponent::RequestPlaySkill(uint32 SkillId)
{
	if (!CanPlaySkill(SkillId))
		return;

	TArray<TWeakObjectPtr<AActor>> Targets;

	FindTarget(SkillId, Targets, true);
	//TODO Target이 없었을때도 예외 처리

	FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillId);
	if (!SkillTableRow)
		return;

	check(SkillTableRow);

	if (!Targets.IsEmpty() || SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::NoneTarget)
	{
		FPC_SkillInfo SkillInfo;
		InitSkillInfo(SkillId, Targets, SkillInfo);

		PlaySkill(SkillInfo);
	}
}

void UPC_SkillComponent::FindTarget(uint32 SkillId, TArray<TWeakObjectPtr<AActor>>& Targets, bool sort)
{
	TArray<AActor*> Enemys;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Enemys);

	FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillId);

	if (!SkillTableRow)
	{
		return;
	}

	const float SkillRange = SkillTableRow->SkillRange;
	const FVector OwnerCharacterPos = OwnerCharacter->GetActorLocation();
	
	if(Enemys.IsEmpty() == false && sort)
	{
		Algo::Sort(Enemys, [&](const AActor* A, const AActor* B)
		{
			return FVector::DistSquared(A->GetActorLocation(), OwnerCharacterPos) <
				FVector::DistSquared(B->GetActorLocation(), OwnerCharacterPos);
		});
	}
	
	for (AActor* Enemy : Enemys)
	{
		if (Enemy != GetOwner())
		{
			if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(Enemy))
			{
				if (CharacterInterface->IsDead())
					continue;
			}

			//스킬 발동자가 Player가 아니면 => 몹이 썼다면
			if (!OwnerCharacter->Implements<UPC_PlayerCharacterInterface>())
			{
				//타겟이 Player가아니면 통과~ 
				if (!Enemy->Implements<UPC_PlayerCharacterInterface>())
				{
					continue;
				}
			}

			if (FVector::Dist(OwnerCharacterPos, Enemy->GetActorLocation()) <= SkillRange)
			{
				Targets.Add(Enemy);
			}
		}
	}

}

bool UPC_SkillComponent::CanPlaySkill(uint32 SkillId)
{
	IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwnerCharacter);
	check(CharacterInterface);

	if (CharacterInterface->IsDead())
		return false;

	return true;
}

void UPC_SkillComponent::PlaySkill(FPC_SkillInfo& SkillInfo)
{
	CurrentPlayingSkillInfos.Add(SkillInfo);

	OnStartSkillDelegate.Broadcast(SkillInfo.SkillDataId);
}

void UPC_SkillComponent::InitSkillInfo(uint32 SkillId, TArray<TWeakObjectPtr<AActor>> Targets, FPC_SkillInfo& SkillInfo)
{
	SkillInfo.SkillDataId = SkillId;
	SkillInfo.Targets = Targets;
	SkillInfo.SkillStartPos = OwnerCharacter->GetActorLocation();
	SkillInfo.SkillStartRot = OwnerCharacter->GetActorRotation();

	float SkillLifeTime = 0.f;
	TArray<FPC_ExecInfo> ExecInfos;
	//미리 초기화 갯수 셋팅
	ExecInfos.AddDefaulted(FPC_GameUtil::GetSkillData(SkillId)->ExecDatas.Num());

	CalcSkillTime(SkillId, SkillLifeTime, ExecInfos);

	SkillInfo.LifeTime = SkillLifeTime;
	SkillInfo.ElapsedTime = 0.f;
	SkillInfo.ExecInfos = ExecInfos;
}

void UPC_SkillComponent::CalcSkillTime(uint32 SkillId, float& SkillLifeTime, TArray<FPC_ExecInfo>& ExecInfos)
{
	//
	for (int32 i = 0; i < ExecInfos.Num(); i++)
	{
		FPC_ExecData& ExecData = FPC_GameUtil::GetSkillData(SkillId)->ExecDatas[i];
		FPC_ExecInfo& ExecInfo = ExecInfos[i];
		ExecInfo.ExecData = &ExecData;
		ExecInfo.ExecSequence = i;

		//인덱스 0은 즉시 실행
		ExecInfo.AnimStartTime = SkillLifeTime;
		ExecInfo.ExecStartTime = SkillLifeTime + ExecData.DelayTime;
		ExecInfo.EndTime = ExecInfo.ExecStartTime + FPC_GameUtil::GetExecData(ExecData.ExecDataId)->Duration;

		//Delay + 스킬 Duration
		SkillLifeTime += ExecData.DelayTime + FPC_GameUtil::GetExecData(ExecData.ExecDataId)->Duration;
	}
}

void UPC_SkillComponent::PlayDecal(uint32 ExecDataId, FVector StartPos, FVector LaunchVel, FRotator Rot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecDataId);
	if (ExecTableRow == nullptr)
		return;

	UMaterialInterface* SkillDecalMaterial = ExecTableRow->SkillDecalMaterial;
	if (SkillDecalMaterial == nullptr)
		return;

	UWorld* World = GetWorld();
	if (!World) return;

	FHitResult Hit;
	FVector End = StartPos;
	End.Z -= 1000.f;

	FCollisionQueryParams Query;
	Query.AddIgnoredActor(OwnerCharacter.Get());

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	if (World->LineTraceSingleByObjectType(Hit, StartPos, End, ObjectQueryParams, Query))
	{
		DrawDebugSphere(World, Hit.ImpactPoint, 20.f, 10, FColor::Red, false, 3.f);

		FRotator DecalRotation(-90.f, GetOwner()->GetActorRotation().Yaw, 0.f);
		FVector MidPoint = (StartPos + Hit.ImpactPoint) * 0.5f;
		FVector DecalLocation = MidPoint + GetOwner()->GetActorForwardVector() * ExecTableRow->DecalSize.Z;

		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
			GetWorld(),
			SkillDecalMaterial,
			ExecTableRow->DecalSize,
			DecalLocation,
			DecalRotation);
		//10.0f);

		if (Decal)
		{
			// FadeOut 설정: 1초 대기 후, 0.5초 동안 페이드 아웃, 완료 시 데칼 파괴
			Decal->SetFadeOut(2.0f, 1.f, true);
		}
	}
}

void UPC_SkillComponent::ProcessSkill(float DeltaTime, FPC_SkillInfo& SkillInfo)
{
	float ElapsedTime = SkillInfo.ElapsedTime;
	for (FPC_ExecInfo& ExecInfo : SkillInfo.ExecInfos)
	{
		if (ExecInfo.bExecFinished)
			continue;

		FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillInfo.SkillDataId);
		check(SkillTableRow);

		FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
		check(ExecTableRow);

		//아직 시작하지 않은 스킬이면
		if (ElapsedTime > ExecInfo.AnimStartTime && !ExecInfo.bAimStarted)
		{
			UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
			check(AnimInstance);

			ExecInfo.bAimStarted = true;

			AnimInstance->StopAllMontages(0.1f);
			AnimInstance->Montage_Play(ExecTableRow->SkillAnim);
		}

		if (ElapsedTime >= ExecInfo.ExecStartTime && ElapsedTime < ExecInfo.EndTime)
		{
			ExecInfo.ElapsedTime += DeltaTime;

			if (!ExecInfo.bExecStarted)
			{
				ExecInfo.bExecStarted = true;
				OnStartExec(SkillInfo, ExecInfo);
			}

			if (SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::NoneTarget)
			{
				ProcessNonTargetExec(DeltaTime, ExecInfo, SkillInfo.SkillStartPos, SkillInfo.SkillStartRot);
			}
			else if (SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::ChainAttack)
			{
				ProcessChainAttackExec(DeltaTime, SkillInfo, ExecInfo, SkillInfo.SkillStartPos,
				                       SkillInfo.SkillStartRot);
			}
			else if (SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::Multiple)
			{
				ProcessMultipleExec(DeltaTime, SkillInfo, ExecInfo, SkillInfo.SkillStartPos, SkillInfo.SkillStartRot);
			}
			else if (SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::TargetPlayer)
			{
				ProcessTargetPlayerExec(DeltaTime, SkillInfo, ExecInfo, SkillInfo.SkillStartPos,
				                        SkillInfo.SkillStartRot);
			}
		}

		if (ExecInfo.EndTime <= ElapsedTime)
		{
			ExecInfo.bExecFinished = true;
			OnEndExec(SkillInfo, ExecInfo);
		}
	}
}

void UPC_SkillComponent::ProcessNonTargetExec(float DeltaTime, FPC_ExecInfo& ExecInfo, FVector StartPos,
                                              FRotator StartRot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	if (ExecTableRow->ExecType == EPC_ExecType::ArcProjectile)
	{
		UWorld* World = GetWorld();
		if (!World) return;

		if (!ExecInfo.bExecCollisionSpawned)
		{
			ExecInfo.bExecCollisionSpawned = true;

			FPC_SkillObjectTableRow* ObjRow = FPC_GameUtil::GetSkillObjectData(ExecTableRow->ExecProperty_0);
			check(ObjRow);

			UClass* ObjClass = ObjRow->SkillObjectActor;
			check(ObjClass);

			USkeletalMeshComponent* Skel = OwnerCharacter->GetMesh();
			check(Skel);

			const APlayerController* PlayerController = CastChecked<APlayerController>(OwnerCharacter->GetController());
			check(PlayerController);

			const FVector SpawnLoc = (ExecTableRow->SkillPosBoneName != NAME_None)
				                         ? Skel->GetSocketLocation(ExecTableRow->SkillPosBoneName)
				                         : OwnerCharacter->GetActorLocation();

			FRotator SpawnRot = OwnerCharacter->GetOwner()->GetActorRotation();
			SpawnRot += ExecTableRow->ProjectileAdditiveRot;

			FTransform Transform;
			Transform.SetLocation(SpawnLoc + ExecTableRow->ProjectileAdditivePos);
			Transform.SetRotation(SpawnRot.Quaternion());

			APC_SkillObject* Obj = CreateSkillObject(Transform, *ObjClass, *ExecTableRow);
			if (Obj != nullptr)
			{
				ExecInfo.SpawnedSkillObject = Obj;
				Obj->OwnerCharacter = OwnerCharacter.Get();
				Obj->SkillObjectId = ExecTableRow->ExecProperty_0;
				FVector Location = SpawnLoc;
				FRotator Rotation = SpawnRot;
				if (IPC_PlayerCharacterInterface* PlayerCharacterInterface = Cast<IPC_PlayerCharacterInterface>(
					GetOwner()))
				{
					if (UPC_ArcSplinePreviewComponent* ArcSplinePreview = PlayerCharacterInterface->
						GetArcSplinePreviewComponent())
					{
						FVector StartVel = ArcSplinePreview->GetStartVelocityPoint();
						if (StartVel != FVector::ZeroVector)
						{
							Obj->FinishSpawning(Transform);
							if (UProjectileMovementComponent* PM = Obj->FindComponentByClass<
								UProjectileMovementComponent>())
							{
								PM->Velocity = StartVel;
								//Obj->PlayImpactPointDecal();
							}
						}
					}
				}
			}
		}
	}
	else if (ExecTableRow->ExecType == EPC_ExecType::Dash)
	{
		float DashRange = ExecTableRow->ExecProperty_0; //Range
		float Duration = ExecTableRow->Duration;

		float CurveAlpha = ExecInfo.ElapsedTime / Duration;
		float PosAlpha = CurveAlpha;

		if (ExecTableRow->ExeCurve)
			PosAlpha = ExecTableRow->ExeCurve->GetFloatValue(CurveAlpha);

		//float Speed =  DashRange / Duration;

		FVector CurrentPos = OwnerCharacter->GetActorLocation();
		FVector NewPos = ExecInfo.ExecStartPos + ExecInfo.ExecStartRot.Vector() * DashRange * PosAlpha;
		NewPos.Z = CurrentPos.Z;

		NewPos = FPC_GameUtil::FindSurfacePos(OwnerCharacter.Get(), NewPos);
		OwnerCharacter->SetActorLocation(NewPos);
	}
	else if (ExecTableRow->ExecType == EPC_ExecType::Dot)
	{
		FVector CurrentPos = FVector::ZeroVector;
		if (ExecTableRow->SkillPosBoneName != NAME_None)
		{
			USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
			check(SkeletalMeshComponent);

			CurrentPos = SkeletalMeshComponent->GetSocketLocation(ExecTableRow->SkillPosBoneName);
		}
		else
		{
			CurrentPos = OwnerCharacter->GetActorLocation();
		}

		FRotator DotDir = OwnerCharacter->GetControlRotation();
		ExecInfo.IntervalElapsedTime += DeltaTime;

		float Interval = ExecTableRow->ExecProperty_0;

		if (ExecTableRow->bSpawnCollision && ExecInfo.IntervalElapsedTime >= Interval && !ExecInfo.
			bExecCollisionSpawned)
		{
			ExecInfo.IntervalElapsedTime = 0.f;

			FVector ExecCollisionPos = CurrentPos + DotDir.Vector() * ExecTableRow->ExecCollisionProperty_2;
			FRotator ExecCollisionRot = DotDir;

			EPC_ExecCollisionType CollisionType = ExecTableRow->ExecCollisionType;
			if (CollisionType == EPC_ExecCollisionType::Box)
			{
				float BoxHeight = ExecTableRow->ExecCollisionProperty_0;
				float BoxWidth = ExecTableRow->ExecCollisionProperty_1;
				float BoxLength = ExecTableRow->ExecCollisionProperty_2;

				FCollisionShape CollisionShape = FCollisionShape::MakeBox(FVector(BoxLength, BoxWidth, BoxHeight));
				CheckCollision(ExecInfo, CollisionShape, ExecCollisionPos, ExecCollisionRot);
			}
		}
	}
	else if (ExecTableRow->ExecType == EPC_ExecType::Pushback)
	{
		if (ExecTableRow->bSpawnCollision && !ExecInfo.bExecCollisionSpawned)
		{
			ExecInfo.bExecCollisionSpawned = true;

			FVector ExecCollisionPos = FVector::ZeroVector;
			if(ExecTableRow->SkillPosBoneName != NAME_None)
			{
				ExecCollisionPos = FPC_GameUtil::GetSocketTransform(
					OwnerCharacter.Get(),ExecTableRow->SkillPosBoneName).GetLocation();
			}
			else
			{
				ExecCollisionPos = ExecInfo.ExecStartPos;
			}
			
			TArray<FOverlapResult> OverlapResults;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(OwnerCharacter.Get());
			
			EPC_ExecCollisionType CollisionType = ExecTableRow->ExecCollisionType;
			FCollisionShape CollisionShape;
			if (CollisionType == EPC_ExecCollisionType::Sphere)
			{
				float SphereRadius = ExecTableRow->ExecCollisionProperty_0;
				CollisionShape = FCollisionShape::MakeSphere(SphereRadius);
			}
			else if(CollisionType == EPC_ExecCollisionType::Box)
			{
				float BoxH = ExecTableRow->ExecCollisionProperty_0;
				float BoxW = ExecTableRow->ExecCollisionProperty_1;
				float BoxL = ExecTableRow->ExecCollisionProperty_2;

				CollisionShape = FCollisionShape::MakeBox(FVector(BoxL, BoxW,BoxH));
			}
			FRotator ExecCollisionRot = ExecInfo.ExecStartRot;
			CheckCollision(ExecInfo, CollisionShape, ExecCollisionPos, ExecCollisionRot);
		}
	}
	else if (ExecTableRow->ExecType == EPC_ExecType::FireCircularRain)
	{
		float GrowthUp = ExecInfo.ExecSequence * 100.f; //적당하게
		float Radius = ExecTableRow->ExecProperty_1 + GrowthUp;
		float SpawnCount = ExecTableRow->ExecProperty_2;
		float IntervalTime = ExecTableRow->Duration / SpawnCount;

		const float SpawnHeight = ExecTableRow->ExecCollisionProperty_0; // 공중 높이
		const float StartAngleDeg = ExecTableRow->ExecCollisionProperty_1; //첫 projectile 각도
		bool bClockwise = true;

		ExecInfo.IntervalElapsedTime += DeltaTime;
		if (ExecInfo.IntervalElapsedTime >= IntervalTime)
		{
			ExecInfo.IntervalElapsedTime = 0.f;

			uint32 StepIndex = ExecInfo.SpawnedCount;
			const float AngleStepDeg = 360.f / SpawnCount; // 각도 분할
			float AngleDeg = StartAngleDeg + (bClockwise ? -1.f : +1.f) * (AngleStepDeg * StepIndex);

			const FVector Center = OwnerCharacter->GetActorLocation();
			const float Rad = FMath::DegreesToRadians(AngleDeg);
			const FVector2D Unit2D = FVector2D(FMath::Cos(Rad), FMath::Sin(Rad));
			FVector Location = Center + FVector(Unit2D.X * Radius, Unit2D.Y * Radius,
			                                    OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() +
			                                    SpawnHeight);

			//땅을 바라보게
			const FRotator Rotation = FRotationMatrix::MakeFromX(FVector::DownVector).Rotator();

			FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(
				ExecTableRow->ExecProperty_0);
			check(SkillObjectTableRow);

			UClass* SkillObjectClass = SkillObjectTableRow->SkillObjectActor;
			check(SkillObjectClass);

			FTransform Transform;
			Transform.SetLocation(Location);
			Transform.SetRotation(Rotation.Quaternion());

			APC_SkillObject* SkillObject = GetWorld()->SpawnActorDeferred<APC_SkillObject>(
				SkillObjectClass, Transform, GetOwner(),
				nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			SkillObject->OwnerCharacter = OwnerCharacter.Get();
			SkillObject->SkillObjectId = ExecTableRow->ExecProperty_0;
			SkillObject->FinishSpawning(Transform);

			// 다음 각도로 진행
			ExecInfo.SpawnedCount++;
			if (ExecInfo.SpawnedCount >= SpawnCount)
			{
				ExecInfo.bExecFinished = true;
			}
		}
	}
	else if (ExecTableRow->ExecType == EPC_ExecType::Projectile)
	{
		if (!ExecInfo.bExecCollisionSpawned)
		{
			ExecInfo.bExecCollisionSpawned = true;

			FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(
				ExecTableRow->ExecProperty_0);
			check(SkillObjectTableRow);

			UClass* SkillObjectClass = SkillObjectTableRow->SkillObjectActor;
			check(SkillObjectClass);

			USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
			check(SkeletalMeshComponent);

			FVector Location = FVector::ZeroVector;
			if (ExecTableRow->SkillPosBoneName != NAME_None)
			{
				Location = SkeletalMeshComponent->GetSocketLocation(ExecTableRow->SkillPosBoneName);
			}
			else
			{
				Location = OwnerCharacter->GetActorLocation();
			}

			FRotator Rotation = OwnerCharacter->GetActorRotation(); //TargetLocation - Location).Rotation();
			Rotation += ExecTableRow->ProjectileAdditiveRot;
			Location += ExecTableRow->ProjectileAdditivePos;

			FTransform Transform;
			Transform.SetLocation(Location);
			Transform.SetRotation(Rotation.Quaternion());
			APC_SkillObject* SkillObject = GetWorld()->SpawnActorDeferred<APC_SkillObject>(
				SkillObjectClass, Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			SkillObject->OwnerCharacter = OwnerCharacter.Get();
			SkillObject->SkillObjectId = ExecTableRow->ExecProperty_0;
			SkillObject->FinishSpawning(Transform);

			const FVector LaunchVel = OwnerCharacter->GetActorForwardVector() * 50.f; // 방향만 넘김(스피드는 PlayDecal에서 곱함)
			FRotator DecalRotation = OwnerCharacter->GetActorForwardVector().Rotation();
			PlayDecal(ExecInfo.ExecData->ExecDataId, OwnerCharacter->GetActorLocation(), LaunchVel, DecalRotation);
		}
	}
	else if (ExecTableRow->ExecType == EPC_ExecType::SentinelProjectile)
	{
		if (!ExecInfo.bExecCollisionSpawned)
		{
			ExecInfo.bExecCollisionSpawned = true;

			// 1) 스킬 오브젝트 클래스 로드
			FPC_SkillObjectTableRow* SkillObjRow = FPC_GameUtil::GetSkillObjectData(ExecTableRow->ExecProperty_0);
			check(SkillObjRow);

			UClass* SkillObjectClass = SkillObjRow->SkillObjectActor;
			check(SkillObjectClass);

			// 2) 파라미터 읽기 (DT/테이블 맵핑은 프로젝트 규칙대로)
			const int32 SentinelCount = FMath::Max(1, static_cast<int32>(ExecTableRow->ExecProperty_1));
			const float OrbitRadius = ExecTableRow->ExecCollisionProperty_0; // 추천: OrbitRadius
			const float TriggerRange = ExecTableRow->ExecCollisionProperty_1; // 추천: DetectRange
			const float OrbitHeight = ExecTableRow->ExecCollisionProperty_2;
			const float AngularSpeed = ExecTableRow->ExecProperty_2 > 0.f ? ExecTableRow->ExecProperty_2 : 180.f;
			const float IdleLifeTime = ExecTableRow->Duration; // 대기 유지 시간

			//TODO 세밀하게 하고싶으면 데이터로 빼도됨
			const float InitialSpeed = 2000.f;
			const float MaxSpeed = 3000.f;
			const float HomingAccel = 8000.f;
			const float DamageRadius = 10.f; // 근접 판정 반경

			// 3) 분산 스폰
			const FVector BaseLoc = OwnerCharacter->GetActorLocation() + FVector(0, 0, OrbitHeight);

			for (int32 i = 0; i < SentinelCount; ++i)
			{
				const float StartAngleDeg = (360.f / SentinelCount) * i;

				FTransform Xform;
				Xform.SetLocation(BaseLoc);
				Xform.SetRotation(FRotator::ZeroRotator.Quaternion());

				APC_SkillObject* Obj = GetWorld()->SpawnActorDeferred<APC_SkillObject>(
					SkillObjectClass, Xform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

				Obj->OwnerCharacter = OwnerCharacter.Get();
				Obj->SkillObjectId = ExecTableRow->ExecProperty_0;

				// Sentinel 전용 초기화 전달
				if (auto* Sentinel = Cast<APC_SentinelProjectile>(Obj))
				{
					FPC_SentinelParams Params;
					Params.OrbitRadius = OrbitRadius;
					Params.OrbitHeight = OrbitHeight;
					Params.AngularSpeed = AngularSpeed;
					Params.TriggerRange = TriggerRange;
					Params.DamageRadius = DamageRadius;

					Params.InitialSpeed = InitialSpeed;
					Params.MaxSpeed = MaxSpeed;
					Params.HomingAccel = HomingAccel;
					Params.bHoming = true;

					Params.IdleLifeTime = IdleLifeTime; // 
					Params.IdleLifeTime = ExecTableRow->Duration; // 발사 후 생존 시간 용도로 사용
					Params.Damage = ExecTableRow->Damage;
					Params.StartAngleDeg = StartAngleDeg;

					Sentinel->InitSentinel(Params);
				}

				Obj->FinishSpawning(Xform);
			}
		}
	}
}

void UPC_SkillComponent::ProcessChainAttackExec(float DeltaTime, FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo,
                                                FVector StartPos, FRotator StartRot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	UWorld* World = GetWorld();
	check(World);

	TArray<TWeakObjectPtr<AActor>>& Targets = SkillInfo.Targets;
	uint32 TargetIndex = ExecInfo.ExecSequence % Targets.Num();
	TWeakObjectPtr<ACharacter> Target = Cast<ACharacter>(Targets[TargetIndex]);
	if (!Target.IsValid())
		return;

	if (ExecTableRow->ExecType == EPC_ExecType::DashToTarget)
	{
		FVector TargetPos = ExecInfo.ExecEndPos;
		FVector CurrentPos = OwnerCharacter->GetActorLocation();

		FVector ToTargetVector = TargetPos - ExecInfo.ExecStartPos;
		FVector ToTargetDir = ToTargetVector.GetSafeNormal();

		float ToTargetLength = (TargetPos - ExecInfo.ExecStartPos).Length();
		float Duration = ExecTableRow->Duration;

		float CurveAlpha = ExecInfo.ElapsedTime / Duration;
		float PosAlpha = CurveAlpha;
		if (ExecTableRow->ExeCurve)
			PosAlpha = ExecTableRow->ExeCurve->GetFloatValue(CurveAlpha);

		//PosAlpha = DeltaTime;

		FVector NewPos = ExecInfo.ExecStartPos + ToTargetDir * ToTargetLength * PosAlpha;
		NewPos.Z = CurrentPos.Z;

		NewPos = FPC_GameUtil::FindSurfacePos(OwnerCharacter.Get(), NewPos);

		OwnerCharacter->SetActorLocation(NewPos);
		OwnerCharacter->SetActorRotation(ToTargetDir.GetSafeNormal2D().Rotation());

		if (CurveAlpha > 0.5f && !ExecInfo.bExecCollisionSpawned)
		{
			ExecInfo.bExecCollisionSpawned = true;

			FVector ExecCollisionPos = ExecInfo.ExecStartPos + ToTargetDir * ToTargetLength / 2.f;
			FRotator ExecCollisionRot = ToTargetDir.Rotation();

			EPC_ExecCollisionType CollisionType = ExecTableRow->ExecCollisionType;
			if (CollisionType == EPC_ExecCollisionType::Box)
			{
				float BoxHeight = ExecTableRow->ExecCollisionProperty_0;
				float BoxWidth = ExecTableRow->ExecCollisionProperty_1;
				float BoxLength = (TargetPos - ExecInfo.ExecStartPos).Length() / 2.f;

				FCollisionShape CollisionShape = FCollisionShape::MakeBox(FVector(BoxLength, BoxWidth, BoxHeight));
				CheckCollision(ExecInfo, CollisionShape, ExecCollisionPos, ExecCollisionRot);
			}
		}
	}
}

void UPC_SkillComponent::ProcessMultipleExec(float DeltaTime, FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo,
                                             FVector StartPos, FRotator StartRot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	if (ExecTableRow->ExecType == EPC_ExecType::Projectile)
	{
		if (!ExecInfo.bExecCollisionSpawned)
		{
			ExecInfo.bExecCollisionSpawned = true;

			FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(
				ExecTableRow->ExecProperty_0);
			check(SkillObjectTableRow);

			UClass* SkillObjectClass = SkillObjectTableRow->SkillObjectActor;
			check(SkillObjectClass);

			TArray<TWeakObjectPtr<AActor>>& Targets = SkillInfo.Targets;

			for (TWeakObjectPtr<AActor> Actor : Targets)
			{
				FVector TargetLocation = Actor->GetActorLocation();

				USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
				check(SkeletalMeshComponent);

				FVector Location = FVector::ZeroVector;
				if (ExecTableRow->SkillPosBoneName != NAME_None)
				{
					Location = FPC_GameUtil::GetSocketTransform(OwnerCharacter.Get(),ExecTableRow->SkillPosBoneName).GetLocation();//SkeletalMeshComponent->GetSocketLocation(ExecTableRow->SkillPosBoneName);
				}
				else
				{
					Location = OwnerCharacter->GetActorLocation();
				}

				FRotator Rotation = (TargetLocation - Location).Rotation();

				FTransform Transform;
				Transform.SetLocation(Location);
				Transform.SetRotation(Rotation.Quaternion());

				APC_SkillObject* SkillObject = GetWorld()->SpawnActorDeferred<APC_SkillObject>(
					SkillObjectClass, Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				SkillObject->OwnerCharacter = OwnerCharacter.Get();
				SkillObject->SkillObjectId = ExecTableRow->ExecProperty_0;
				SkillObject->FinishSpawning(Transform);
			}
		}
	}
}

void UPC_SkillComponent::ProcessTargetPlayerExec(float DeltaTime, FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo,
                                                 FVector StartPos, FRotator StartRot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	if (ExecTableRow->ExecType == EPC_ExecType::FireMultipleProjectile)
	{
		float LoopCount = ExecTableRow->ExecProperty_1;
		float IntervalTime = ExecTableRow->Duration / LoopCount;

		ExecInfo.IntervalElapsedTime += DeltaTime;
		if (ExecInfo.IntervalElapsedTime >= IntervalTime)
		{
			ExecInfo.IntervalElapsedTime = 0.f;

			FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(
				ExecTableRow->ExecProperty_0);
			check(SkillObjectTableRow);

			UClass* SkillObjectClass = SkillObjectTableRow->SkillObjectActor;
			check(SkillObjectClass);

			TArray<TWeakObjectPtr<AActor>>& Targets = SkillInfo.Targets;
			if (Targets.IsEmpty())
				return;

			//해당 스킬은 몬스터가 쓰는 스킬로
			//인덱스 0번이 Player로 보장
			TWeakObjectPtr<AActor> PlayerActor = Targets[0];
			ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerActor.Get());
			if (!PlayerCharacter->Implements<UPC_PlayerCharacterInterface>())
				return;

			FVector TargetLocation = PlayerCharacter->GetActorLocation();

			USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
			check(SkeletalMeshComponent);

			float RandomRadius = ExecTableRow->ExecCollisionProperty_0;
			float SpawnHeight = ExecTableRow->ExecCollisionProperty_1;
			FVector Location = FVector::ZeroVector;

			if (RandomRadius > 0.f)
			{
				//RandPointInBox 
				const FVector RnadPos = FMath::RandPointInBox(FBox(FVector(RandomRadius / 2.f),
				                                                   FVector(RandomRadius / 2.f)));

				//머리 꼭대기에서 얼만큼 위로
				Location = OwnerCharacter->GetActorLocation() + FVector(0.f, 0.f,
				                                                        OwnerCharacter->GetCapsuleComponent()->
				                                                        GetScaledCapsuleHalfHeight() + SpawnHeight);
				Location += RnadPos;
			}
			else
			{
				//랜덤 요소가 없다면
				//스킬 본에서 발사!
				if (ExecTableRow->SkillPosBoneName != NAME_None)
				{
					Location = SkeletalMeshComponent->GetSocketLocation(ExecTableRow->SkillPosBoneName);
				}
				else
				{
					Location = OwnerCharacter->GetActorLocation();
				}
			}

			FRotator Rotation = (TargetLocation - Location).Rotation();

			FTransform Transform;
			Transform.SetLocation(Location);
			Transform.SetRotation(Rotation.Quaternion());

			APC_SkillObject* SkillObject = GetWorld()->SpawnActorDeferred<APC_SkillObject>(
				SkillObjectClass, Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			SkillObject->OwnerCharacter = OwnerCharacter.Get();
			SkillObject->SkillObjectId = ExecTableRow->ExecProperty_0;
			SkillObject->FinishSpawning(Transform);
		}
	}
}

void UPC_SkillComponent::CheckCollision(const FPC_ExecInfo& ExecInfo, FCollisionShape CollisionShape,
                                        const FVector& Pos, const FRotator& Rot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	if (ExecTableRow->bSpawnCollision == false)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	UWorld* World = GetWorld();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter.Get());
	
	if (FPC_GameUtil::IsDebugDrawing(OwnerCharacter.Get()))
		DrawDebugBox(World, Pos, CollisionShape.GetExtent(), Rot.Quaternion(), FColor::Red, false);

	if (APC_BaseCharacter* BaseCharacter = Cast<APC_BaseCharacter>(OwnerCharacter.Get()))
	{
		//몬스터도 스킬을 사용하기 떄문에 처리 필요
		QueryParams.AddIgnoredActor(OwnerCharacter.Get());
		if (World->OverlapMultiByChannel(OverlapResults, Pos, Rot.Quaternion(),
		                                 FPC_GameUtil::GetAttackCollisionChannel(BaseCharacter->CharacterDataID),
		                                 CollisionShape, QueryParams))
		{
			for (FOverlapResult& OverlapResult : OverlapResults)
			{
				if (ACharacter* HitCharacter = Cast<ACharacter>(OverlapResult.GetActor()))
				{
					FDamageEvent DamageEvent;
					HitCharacter->TakeDamage(ExecTableRow->Damage, DamageEvent,
					                         OwnerCharacter->GetController(), OwnerCharacter.Get());

					if (ExecTableRow->CameraShakeAction == EPC_CameraShakeActionType::OnHit)
					{
						FPC_GameUtil::CameraShake(ExecTableRow->ShakeMagnitude);
					}

					if (ExecTableRow->HitFX_Niagara)
					{
						FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->HitFX_Niagara,
						                                    ExecInfo.ExecStartPos,
						                                    ExecInfo.ExecStartRot, ExecTableRow->HitEffectScale);
					}

					if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(
						OverlapResult.GetActor()))
					{
						UPC_CrowdControlComponent* CrowdControlComponent = CharacterInterface->
							GetCrowdControlComponent();
						check(CrowdControlComponent);

						const float CrowdControlId = ExecTableRow->CrowdControlId;
						if (CrowdControlId > INDEX_NONE)
						{
							CrowdControlComponent->RequestPlayerCC(CrowdControlId, GetOwner());
						}
					}
				}
			}
		}

		if (ExecTableRow->CameraShakeAction == EPC_CameraShakeActionType::Always)
			FPC_GameUtil::CameraShake(ExecTableRow->ShakeMagnitude);
	}
}

void UPC_SkillComponent::OnStartExec(FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	USkeletalMeshComponent* SkeletalMesh = OwnerCharacter->GetMesh();
	check(SkeletalMesh);

	ExecInfo.ExecStartPos = OwnerCharacter->GetActorLocation();
	ExecInfo.ExecStartRot = OwnerCharacter->GetActorRotation();

	if (ExecTableRow->SkillFxAttachType == EPC_SkillFxAttachType::AttachToBone)
	{
		ExecInfo.AttachedFx = FPC_GameUtil::SpawnEffectAttached(ExecTableRow->ExecFX_Niagara_Start, SkeletalMesh,
		                                                        ExecTableRow->SkillPosBoneName, FVector::ZeroVector,
		                                                        FRotator::ZeroRotator, EAttachLocation::SnapToTarget,
		                                                        false);
	}
	else if (ExecTableRow->SkillFxAttachType == EPC_SkillFxAttachType::AttachToMiddlePos)
	{
		//		IPC_CharacterInterface* CharacterInterface =CastChecked<IPC_CharacterInterface>(OwnerCharacter);
		//		ExecInfo.AttachedFx = FPC_GameUtil::SpawnEffectAttached(ExecTableRow->ExecFX_Niagara_Start, 
		//CharacterInterface->getmid)
	}
	else if (ExecTableRow->SkillFxAttachType == EPC_SkillFxAttachType::Surface)
	{
		FVector FXSpawnPos;
		if(ExecTableRow->SkillPosBoneName != NAME_None)
		{
			FXSpawnPos = FPC_GameUtil::GetSocketTransform(OwnerCharacter.Get(), ExecTableRow->SkillPosBoneName).
				GetLocation();
		}
		else
		{
			FXSpawnPos = ExecInfo.ExecStartPos;
		}

		//위에서 아래로
		UWorld* World = GetWorld();
		FVector TraceStartPos = FXSpawnPos + FVector(0, 0, 300.f);
		FVector TraceEndPos = FXSpawnPos - FVector(0, 0, 1000.f);

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		
		FHitResult HitResult;
		World->LineTraceSingleByObjectType(HitResult, TraceStartPos, TraceEndPos, ObjectQueryParams);

		FXSpawnPos = HitResult.ImpactPoint;

		FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Niagara_Start, FXSpawnPos, ExecInfo.ExecStartRot, 1.f);
		FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Cascade_Start, FXSpawnPos, ExecInfo.ExecStartRot, 1.f);
	}
	else
	{
		FVector FXSpawnPos;
		if (ExecTableRow->SkillPosBoneName != NAME_None)
		{
			FXSpawnPos = FPC_GameUtil::GetSocketTransform(OwnerCharacter.Get(), ExecTableRow->SkillPosBoneName).GetLocation();
		}
		else
		{
			FXSpawnPos = ExecInfo.ExecStartPos;
		}
		
		FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Niagara_Start, FXSpawnPos, ExecInfo.ExecStartRot);
		FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Cascade_Start, FXSpawnPos, ExecInfo.ExecStartRot);
	}
	
	if (ExecTableRow->ExecType == EPC_ExecType::DashToTarget)
	{
		TArray<TWeakObjectPtr<AActor>>& Targets = SkillInfo.Targets;
		if (Targets.IsEmpty())
			return;

		//TargetIndex가 시퀀스가 개수보다 안 크게
		uint32 TargetIndex = ExecInfo.ExecSequence % Targets.Num();
		TWeakObjectPtr<ACharacter> Target = Cast<ACharacter>(Targets[TargetIndex]);

		if (!Target.IsValid())
			return;

		FVector TargetPos = Target->GetActorLocation();

		FVector ToTargetVector = TargetPos - ExecInfo.ExecStartPos;
		FVector ToTargetDir = ToTargetVector.GetSafeNormal();

		//얼만큼 지나쳐 도착할것인가.
		float OverRunDistance = ExecTableRow->ExecProperty_0;
		TargetPos += ToTargetDir * OverRunDistance;

		ExecInfo.ExecEndPos = TargetPos;
		ExecInfo.ExecEndRot = ToTargetDir.GetSafeNormal2D().Rotation();
	}
}

void UPC_SkillComponent::OnEndExec(FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	USkeletalMeshComponent* SkeletalMesh = OwnerCharacter->GetMesh();
	check(SkeletalMesh);

	const FVector Location = OwnerCharacter->GetActorLocation();
	const FRotator Rotation = OwnerCharacter->GetActorRotation();

	FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Niagara_End, Location, Rotation);
	FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Cascade_End, Location, Rotation);

	if (ExecInfo.AttachedFx)
	{
		ExecInfo.AttachedFx->Deactivate(); //Deactivate : 
	}
}

APC_SkillObject* UPC_SkillComponent::CreateSkillObject(const FTransform Transform, UClass& SkillObj, FPC_ExecTableRow& ExecTableRow)
{
	UWorld* World = GetWorld();
	check(World);
	
	APC_SkillObject* Obj = World->SpawnActorDeferred<APC_SkillObject>(
			&SkillObj, Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if(ExecTableRow.CrowdControlId > 0)
	{
		const int32 CrowdControlId = ExecTableRow.CrowdControlId;
		FPC_OnBeginOverlap Delegate;
		Delegate.AddLambda(
			[Obj, CrowdControlId](AActor* HitActor)
			{
				if (!Obj)
					return;

				if (!IsValid(HitActor))
					return;

				if(IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(HitActor))
				{
					UPC_CrowdControlComponent* CrowdControlComponent = CharacterInterface->
						GetCrowdControlComponent();
					check(CrowdControlComponent);
					//스킬오브젝트 기준으로 
					CrowdControlComponent->RequestPlayerCC(CrowdControlId, Obj);
				}
				FPC_GameUtil::AddOnScreenDebugMessage("피격!!!!!!!!!!!");
			});

		Obj->SetCrowdControl(Delegate);
	}
	
	return Obj;
}

void UPC_SkillComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Tick_PlaySkill(DeltaTime);
}

void UPC_SkillComponent::Tick_PlaySkill(float DeltaTime)
{
	TArray<FPC_SkillInfo> SkillToCoolDown;
	for (FPC_SkillInfo& SkillInfo : CurrentPlayingSkillInfos)
	{
		FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillInfo.SkillDataId);
		check(SkillTableRow);

		SkillInfo.ElapsedTime += DeltaTime;
		ProcessSkill(DeltaTime, SkillInfo);

		if (SkillInfo.ElapsedTime > SkillInfo.LifeTime)
		{
			SkillToCoolDown.Add(SkillInfo);

			OnEndSkillDelegate.Broadcast(SkillInfo.SkillDataId);
		}
	}

	//쿨다운 스킬 add
	for (FPC_SkillInfo& SkillInfo : SkillToCoolDown)
	{
		CoolDownSkillInfos.Add(SkillInfo);
	}

	CurrentPlayingSkillInfos.RemoveAll([&](const FPC_SkillInfo& SkillInfo)
	{
		return SkillInfo.ElapsedTime > SkillInfo.LifeTime;
	});
}
