#include "PC_SkillComponent.h"

#include <tiffio.h>

#include "Camera/CameraComponent.h"
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
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Enemys);

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

		//인덱스 0은 즉시 실행
		ExecInfo.AnimStartTime = SkillLifeTime;
		ExecInfo.ExecStartTime = SkillLifeTime + ExecData.DelayTime;
		ExecInfo.EndTime = ExecInfo.ExecStartTime + FPC_GameUtil::GetExecData(ExecData.ExecDataId)->Duration;

		//Delay + 스킬 Duration
		SkillLifeTime += ExecData.DelayTime + FPC_GameUtil::GetExecData(ExecData.ExecDataId)->Duration;
	}
	
}

void UPC_SkillComponent::ProcessNonTargetSkill(float DeltaTime, FPC_SkillInfo& SkillInfo)
{
	float ElapsedTime = SkillInfo.ElapsedTime;
	for(FPC_ExecInfo& ExecInfo : SkillInfo.ExecInfos)
	{
		//아직 시작하지 않은 스킬이면
		if(ElapsedTime > ExecInfo.AnimStartTime && !ExecInfo.bAimStarted)
		{
			FPC_ExecTableRow* ExecTableRow = FPC_GameUtil::GetExecData(ExecInfo.ExecData->ExecDataId);
			check(ExecTableRow);

			UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
			check(AnimInstance);

			ExecInfo.bAimStarted = true;

			AnimInstance->StopAllMontages(0.1f);
			AnimInstance->Montage_Play(ExecTableRow->SkillAnim);
		}

		if(ElapsedTime >= ExecInfo.ExecStartTime && ElapsedTime < ExecInfo.EndTime)
		{
			ExecInfo.bExecStarted = true;
			ProcessNonTargetExec(DeltaTime, ExecInfo, SkillInfo.SkillStartPos, SkillInfo.SkillStartRot);
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

		float Speed =  DashRange / Duration;

		FVector CurrentPos = OwnerCharacter->GetActorLocation();
		FVector NewPos = CurrentPos + StartRot.Vector() * Speed * DeltaTime;

		FHitResult HitResult;
		OwnerCharacter->SetActorLocation(NewPos, true, &HitResult);

		//지형에 막혔는지
		if(HitResult.bBlockingHit)
		{
			OwnerCharacter->SetActorLocation(CurrentPos);
		}
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
		if(SkillInfo.ElapsedTime > SkillInfo.LifeTime)
		{
			SkillToCoolDown.Add(SkillInfo);
			continue;
		}

		if(SkillTableRow->SkillTargetingType == EPC_SkillTargetingType::NoneTarget)
		{
			ProcessNonTargetSkill(DeltaTime, SkillInfo);
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

