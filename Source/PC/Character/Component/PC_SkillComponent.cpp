#include "PC_SkillComponent.h"

#include <tiffio.h>

#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PC/Data/PC_CameraDataAsset.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
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

	if(!Targets.IsEmpty() || FPC_GameUtil::GetSkillData(SkillId)->SkillTargetingType == EPC_SkillTargetingType::NoneTarget)
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

	const float SkillRange = FPC_GameUtil::GetSkillData(SkillId)->SkillRange;
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
		
		FHitResult HitResult;
		OwnerCharacter->SetActorLocation(NewPos, true, &HitResult);
		
		//지형에 막혔는지
		if(HitResult.bBlockingHit)
		{
			OwnerCharacter->SetActorLocation(CurrentPos);
		}
	}
}

void UPC_SkillComponent::ProcessChainAttackExec(float DeltaTime, FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo,
	FVector StartPos, FRotator StartRot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	TArray<TWeakObjectPtr<AActor>>& Targets = SkillInfo.Targets;
	uint32 TargetIndex = ExecInfo.ExecSequence % Targets.Num();
	TWeakObjectPtr<ACharacter> Target = Cast<ACharacter>(Targets[TargetIndex]);
	if(!Target.IsValid())
		return;

	if(ExecTableRow->ExecType == EPC_ExecType::DashToTarget)
	{
		FVector TargetPos = Target->GetActorLocation();
		FVector CurrentPos = OwnerCharacter->GetActorLocation();

		FVector ToTargetPos = TargetPos - ExecInfo.ExecStartPos;
		FVector ToTargetDir = ToTargetPos.GetSafeNormal();

		TargetPos += ToTargetDir * 200.0f;

		float ToTargetLength= (TargetPos - ExecInfo.ExecStartPos).Length();
		float Duration = ExecTableRow->Duration;

		float CurveAlpha = ExecInfo.ElapsedTime / Duration;
		float PosAlpha = ExecTableRow->ExeCurve->GetFloatValue(CurveAlpha);

		FVector NewPos = ExecInfo.ExecStartPos + ToTargetDir * ToTargetLength * PosAlpha;

		FHitResult Result;
		OwnerCharacter->SetActorLocation(NewPos, true, &Result);
		//Z값 날리기 pitch
		OwnerCharacter->SetActorRotation(ToTargetDir.GetSafeNormal2D().Rotation());

		//벽에 막히면
		if(Result.bBlockingHit)
		{
			OwnerCharacter->SetActorLocation(CurrentPos);	
		}

		//sweep
		if(CurveAlpha > 0.5f && !ExecInfo.bExecCollisionSpawned)
		{
			FVector ExecCollisionPos = ExecInfo.ExecStartPos + ToTargetDir * ToTargetLength / 2.f;
			FRotator ExecCollisionRot = ToTargetDir.Rotation();
			
			EPC_ExecCollisionType CollisionType = ExecTableRow->ExecCollisionType;
			if(CollisionType == EPC_ExecCollisionType::Box)
			{
				float BoxHeight = ExecTableRow->ExecCollisionProperty_0;
				float BoxWidth = ExecTableRow->ExecCollisionProperty_1;
				float BoxLength = (TargetPos - ExecInfo.ExecStartPos).Length() / 2.f;

				FCollisionShape CollisionShape = FCollisionShape::MakeBox(FVector(BoxLength, BoxWidth,BoxHeight));
				SpawnExecCollsion(ExecInfo, CollisionShape, ExecCollisionPos, ExecCollisionRot);
			}
		}
	}
}

void UPC_SkillComponent::SpawnExecCollsion(const FPC_ExecInfo& ExecInfo, FCollisionShape CollisionShape,
	const FVector& Pos, const FRotator& Rot)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	TArray<FOverlapResult> OverlapResults;
	UWorld* World = GetWorld();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter.Get());
	
	if(World->OverlapMultiByChannel(OverlapResults, Pos, Rot.Quaternion(), ECC_Pawn,
		CollisionShape, QueryParams))
	{
		for(FOverlapResult& OverlapResult : OverlapResults)
		{
			if(ACharacter* HitCharacter = Cast<ACharacter>(OverlapResult.GetActor()))
			{
				FDamageEvent DamageEvent;
				HitCharacter->TakeDamage(ExecTableRow->Damage, DamageEvent,
					OwnerCharacter->GetController(), OwnerCharacter.Get());
			}
		}
	}
}

void UPC_SkillComponent::OnStartExec(FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo)
{
	FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
	check(ExecTableRow);

	ExecInfo.ExecStartPos = OwnerCharacter->GetActorLocation();
	ExecInfo.ExecStartRot = OwnerCharacter->GetActorRotation();
	
}

void UPC_SkillComponent::OnEndExec(FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo)
{
	
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
		if(SkillInfo.ElapsedTime > SkillInfo.LifeTime)
		{
			SkillToCoolDown.Add(SkillInfo);
			continue;
		}

		ProcessSkill(DeltaTime, SkillInfo);
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

