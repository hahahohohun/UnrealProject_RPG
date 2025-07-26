#include "PC_SkillComponent.h"

#include <tiffio.h>

//#include "SAdvancedRotationInputBox.h"
#include "PC_CrowdControlComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Framework/Text/ShapedTextCache.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PC/Data/PC_CameraDataAsset.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
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
	if(!CanPlaySkill(SkillId))
		return;

	TArray<TWeakObjectPtr<AActor>> Targets;
	FindTarget(SkillId, Targets);

	FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillId);
	if(!SkillTableRow)
		return;
	
	check(SkillTableRow);
	
	if(!Targets.IsEmpty() || SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::NoneTarget)
	{
		FPC_SkillInfo SkillInfo;
		InitSkillInfo(SkillId, Targets, SkillInfo);
		
		PlaySkill(SkillInfo);
	}
}

void UPC_SkillComponent::FindTarget(uint32 SkillId, TArray<TWeakObjectPtr<AActor>>& Targets)
{
	TArray<AActor*> Enemys;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Enemys);

	FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillId);

	if(!SkillTableRow)
	{
		return;
	}
	
	const float SkillRange = SkillTableRow->SkillRange;
	const FVector OwnerCharacterPos = OwnerCharacter->GetActorLocation();

	for (AActor* Enemy : Enemys)
	{
		if(Enemy != GetOwner())
		{
			if(FVector::Dist(OwnerCharacterPos, Enemy->GetActorLocation()) <= SkillRange)
			{
				Targets.Add(Enemy);
			}
		}
	}
}

bool UPC_SkillComponent::CanPlaySkill(uint32 SkillId)
{
	return true;
}

void UPC_SkillComponent::PlaySkill(FPC_SkillInfo& SkillInfo)
{
	CurrentPlayingSkillInfos.Add(SkillInfo);
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
	for(int32 i = 0; i < ExecInfos.Num(); i++)
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

void UPC_SkillComponent::ProcessSkill(float DeltaTime, FPC_SkillInfo& SkillInfo)
{
	float ElapsedTime = SkillInfo.ElapsedTime;
	for(FPC_ExecInfo& ExecInfo : SkillInfo.ExecInfos)
	{
		if(ExecInfo.bExecFinished)
			continue;
		
		FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillInfo.SkillDataId);
		check(SkillTableRow);
		
		FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
		check(ExecTableRow);

		//아직 시작하지 않은 스킬이면
		if(ElapsedTime > ExecInfo.AnimStartTime && !ExecInfo.bAimStarted)
		{
			UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
			check(AnimInstance);

			ExecInfo.bAimStarted = true;

			AnimInstance->StopAllMontages(0.1f);
			AnimInstance->Montage_Play(ExecTableRow->SkillAnim);
		}

		if(ElapsedTime >= ExecInfo.ExecStartTime && ElapsedTime < ExecInfo.EndTime)
		{
			ExecInfo.ElapsedTime += DeltaTime;
		
			if(!ExecInfo.bExecStarted)
			{
				ExecInfo.bExecStarted = true;
				OnStartExec(SkillInfo, ExecInfo);
			}

			if(SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::NoneTarget)
			{
				ProcessNonTargetExec(DeltaTime, ExecInfo, SkillInfo.SkillStartPos, SkillInfo.SkillStartRot);	
			}
			else if(SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::ChainAttack)
			{
				ProcessChainAttackExec(DeltaTime, SkillInfo, ExecInfo, SkillInfo.SkillStartPos, SkillInfo.SkillStartRot);
			}
			else if(SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::Multiple)
			{
				ProcessMultipleExec(DeltaTime, SkillInfo, ExecInfo, SkillInfo.SkillStartPos, SkillInfo.SkillStartRot);	
			}
		}

		if(ExecInfo.EndTime <= ElapsedTime)
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

	if(ExecTableRow->ExecType == EPC_ExecType::Dash)
	{
		float DashRange = ExecTableRow->ExecProperty_0; //Range
		float Duration = ExecTableRow->Duration;

		float CurveAlpha = ExecInfo.ElapsedTime / Duration;
		float PosAlpha = ExecTableRow->ExeCurve->GetFloatValue(CurveAlpha);
		
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

		if (ExecTableRow->bSpawnCollision && ExecInfo.IntervalElapsedTime >= Interval && !ExecInfo.bExecCollisionSpawned)
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
	else if(ExecTableRow->ExecType == EPC_ExecType::Pushback)
	{
		if(ExecTableRow->bSpawnCollision && !ExecInfo.bExecCollisionSpawned)
		{
			ExecInfo.bExecCollisionSpawned = true;

			FVector ExecCollisionPos = ExecInfo.ExecStartPos;
			FRotator ExecCollisionRot = ExecInfo.ExecStartRot;
			float SphereRadius = ExecTableRow->ExecCollisionProperty_0;
			FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SphereRadius);

			UWorld* World = GetWorld();
			check(World);

			TArray<FOverlapResult> OverlapResults;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(OwnerCharacter.Get());

			EPC_ExecCollisionType CollisionType = ExecTableRow->ExecCollisionType;
			if(CollisionType == EPC_ExecCollisionType::Sphere)
			{
				CheckCollision(ExecInfo, CollisionShape, ExecCollisionPos, ExecCollisionRot);
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
		FVector TargetPos = Target->GetActorLocation();
		FVector CurrentPos = OwnerCharacter->GetActorLocation();

		FVector ToTargetVector = TargetPos - ExecInfo.ExecStartPos;
		FVector ToTargetDir = ToTargetVector.GetSafeNormal();

		TargetPos += ToTargetDir * 200.f;

		float ToTargetLength = (TargetPos - ExecInfo.ExecStartPos).Length();
		float Duration = ExecTableRow->Duration;

		float CurveAlpha = ExecInfo.ElapsedTime / Duration;
		float PosAlpha = ExecTableRow->ExeCurve->GetFloatValue(CurveAlpha);

		FVector NewPos = ExecInfo.ExecStartPos + ToTargetDir * ToTargetLength * PosAlpha;
		NewPos.Z = CurrentPos.Z;
		
		NewPos = FPC_GameUtil::FindSurfacePos(OwnerCharacter.Get(), NewPos);
		
		OwnerCharacter->SetActorLocation(NewPos);
		OwnerCharacter->SetActorRotation(ToTargetDir.GetSafeNormal2D().Rotation());

		if (CurveAlpha > 0.5f && !ExecInfo.bExecCollisionSpawned)
		{
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

			FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(ExecTableRow->ExecProperty_0);
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
					Location = SkeletalMeshComponent->GetSocketLocation(ExecTableRow->SkillPosBoneName);
				}
				else
				{
					Location = OwnerCharacter->GetActorLocation();
				}
	
				FRotator Rotation = (TargetLocation - Location).Rotation();

				FTransform Transform;
				Transform.SetLocation(Location);
				Transform.SetRotation(Rotation.Quaternion());

				APC_SkillObject* SkillObject = GetWorld()->SpawnActorDeferred<APC_SkillObject>(SkillObjectClass, Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				SkillObject->OwnerCharacter = OwnerCharacter.Get();
				SkillObject->SkillObjectId = ExecTableRow->ExecProperty_0;
				SkillObject->FinishSpawning(Transform);
			}
		}
	}
}

void UPC_SkillComponent::CheckCollision(const FPC_ExecInfo& ExecInfo, FCollisionShape CollisionShape,
                                           const FVector& Pos, const FRotator& Rot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	TArray<FOverlapResult> OverlapResults;
	UWorld* World = GetWorld();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter.Get());

	DrawDebugBox(World, Pos, CollisionShape.GetExtent(), Rot.Quaternion(), FColor::Red, false);
	
	if(World->OverlapMultiByProfile(OverlapResults, Pos, Rot.Quaternion(), TEXT("EnemyPreset"),
		CollisionShape, QueryParams))
	{
		for(FOverlapResult& OverlapResult : OverlapResults)
		{
			if(ACharacter* HitCharacter = Cast<ACharacter>(OverlapResult.GetActor()))
			{
				FDamageEvent DamageEvent;
				HitCharacter->TakeDamage(ExecTableRow->Damage, DamageEvent,
					OwnerCharacter->GetController(), OwnerCharacter.Get());

				if(ExecTableRow->bPlayCameraShake)
				{
					FPC_GameUtil::CameraShake();
				}
				
				if(IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OverlapResult.GetActor()))
				{
					UPC_CrowdControlComponent* CrowdControlComponent = CharacterInterface->GetCrowdControlComponent();
					check(CrowdControlComponent);

					const float CrowdControlId = ExecTableRow->CrowdControlId;
					if(CrowdControlId > INDEX_NONE)
					{
						CrowdControlComponent->RequestPlayerCC(CrowdControlId, GetOwner());
					}
				}
			}
		}
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
	
	if (ExecTableRow->bAttachFx)
	{
		ExecInfo.AttachedFx = FPC_GameUtil::SpawnEffectAttached(ExecTableRow->ExecFX_Niagara_Start, SkeletalMesh, ExecTableRow->SkillPosBoneName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
	}
	else
	{
		FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Niagara_Start, ExecInfo.ExecStartPos, ExecInfo.ExecStartRot);
		FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), ExecTableRow->ExecFX_Cascade_Start, ExecInfo.ExecStartPos, ExecInfo.ExecStartRot);
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

void UPC_SkillComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Tick_PlaySkill(DeltaTime);
}

void UPC_SkillComponent::Tick_PlaySkill(float DeltaTime)
{
	TArray<FPC_SkillInfo> SkillToCoolDown;
	for(FPC_SkillInfo& SkillInfo : CurrentPlayingSkillInfos)
	{
		FPC_SkillTableRow* SkillTableRow = FPC_GameUtil::GetSkillData(SkillInfo.SkillDataId);
		check(SkillTableRow);

		SkillInfo.ElapsedTime += DeltaTime;
		ProcessSkill(DeltaTime, SkillInfo);

		if(SkillInfo.ElapsedTime > SkillInfo.LifeTime)
		{
			SkillToCoolDown.Add(SkillInfo);
		}
	}

	//쿨다운 스킬 add
	for(FPC_SkillInfo& SkillInfo : SkillToCoolDown)
	{
		CoolDownSkillInfos.Add(SkillInfo);
	}

	CurrentPlayingSkillInfos.RemoveAll([&](const FPC_SkillInfo& SkillInfo)
	{
		return SkillInfo.ElapsedTime > SkillInfo.LifeTime;
	});
}

