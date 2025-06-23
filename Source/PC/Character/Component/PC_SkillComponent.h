#pragma once

#include "CoreMinimal.h"
#include "CoreMinimal.h"
#include "PC_SkillComponent.generated.h"


struct FPC_ExecData;

//skill comp에서 내부적에서 일시적으로 만들어지는 객체
struct FPC_ExecInfo
{
	FPC_ExecData* ExecData = nullptr;

	bool bAimStarted = false;
	bool bExecStarted = false;

	float AnimStartTime = 0.0f;
	float ExecStartTime = 0.0f;
	float EndTime = 0.f;
};

struct FPC_SkillInfo
{
	uint32 SkillDataId = 0;
	float LifeTime = 0.f;
	float ElapsedTime = 0.f;

	TArray<TWeakObjectPtr<AActor>> Targets;

	FVector SkillStartPos = FVector::ZeroVector;
	FRotator SkillStartRot = FRotator::ZeroRotator;

	TArray<FPC_ExecInfo> ExecInfos;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_SkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Called when the game starts
	UPC_SkillComponent();

	void Tick_PlaySkill(float DeltaTime);
	
public:
	virtual void BeginPlay() override;

	void RequestPlaySkill(uint32 SkillId);
	void FindTarget(uint32 SkillId, TArray<TWeakObjectPtr<AActor>>& Targets);
	bool CanPlaySkill(uint32 SkillId);
	void PlaySkill(FPC_SkillInfo& SkillInfo);

	void InitSkillInfo(uint32 SkillId, TArray<TWeakObjectPtr<AActor>> Targets, FPC_SkillInfo& SkillInfo);
	void CalcSkillTime(uint32 SkillId, float& SkillLifeTime, TArray<FPC_ExecInfo>& ExecInfos);

	//논타겟
	void ProcessNonTargetSkill(float DeltaTime, FPC_SkillInfo& SkillInfo);
	void ProcessNonTargetExec(float DeltaTime, FPC_ExecInfo& ExecInfo, FVector StartPos, FRotator StartRot);

	
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//실행중인
	TArray<FPC_SkillInfo> CurrentPlayingSkillInfos;
	//쿨타임중인
	TArray<FPC_SkillInfo> CoolDownSkillInfos;
	
	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr;
};

