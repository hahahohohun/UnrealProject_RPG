#pragma once

#include "CoreMinimal.h"
#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "PC_SkillComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPC_OnStartSkillDelegate, uint32, SkillId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPC_OnEndSkillDelegate, uint32, SkillId);

struct FPC_ExecData;

//skill comp에서 내부적에서 일시적으로 만들어지는 객체
struct FPC_ExecInfo
{
	FPC_ExecData* ExecData = nullptr;

	bool bAimStarted = false;
	bool bExecStarted = false;
	bool bExecFinished = false;
	bool bExecCollisionSpawned = false; //한번 콜리전 관련 함수

	//투사체 예측 경로 관련
	bool bPathDrawing = false;
	float PathUpdateAcc = 0.0f;
	TWeakObjectPtr<AActor>   SpawnedSkillObject = nullptr; // 스폰된 투사체
	TArray<FVector>          PathPoints;                   // 경로 캐시
	//
	
	float AnimStartTime = 0.0f;
	float ExecStartTime = 0.0f;
	float EndTime = 0.f;

	float ElapsedTime = 0.f;
	float IntervalElapsedTime = 0.f;
	
	uint32 ExecSequence = 0;
	uint32 SpawnedCount = 0;
	
	FVector ExecStartPos = FVector::ZeroVector;
	FRotator ExecStartRot  = FRotator::ZeroRotator;

	FVector ExecEndPos = FVector::ZeroVector;
	FRotator ExecEndRot  = FRotator::ZeroRotator;
	
	TObjectPtr<UNiagaraComponent> AttachedFx = nullptr;
	
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

	void PlayDecal(uint32 ExecDataId, FVector StartPos, FVector ForwardVector, FRotator Rot );
	
	//논타겟
	void ProcessSkill(float DeltaTime, FPC_SkillInfo& SkillInfo);
	void ProcessNonTargetExec(float DeltaTime, FPC_ExecInfo& ExecInfo, FVector StartPos, FRotator StartRot);
	void ProcessChainAttackExec(float DeltaTime, FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo, FVector StartPos, FRotator StartRot);
	void ProcessMultipleExec(float DeltaTime, FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo, FVector StartPos, FRotator StartRot);
	void ProcessTargetPlayerExec(float DeltaTime, FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo, FVector StartPos, FRotator StartRot);
	
	void CheckCollision(const FPC_ExecInfo& ExecInfo, FCollisionShape CollisionShape, const FVector& Vector, const FRotator& Rotator);
	void OnStartExec(FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo);
	void OnEndExec(FPC_SkillInfo& SkillInfo, FPC_ExecInfo& ExecInfo);

	
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//실행중인
	TArray<FPC_SkillInfo> CurrentPlayingSkillInfos;
	//쿨타임중인
	TArray<FPC_SkillInfo> CoolDownSkillInfos;
	
	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr;

	FPC_OnStartSkillDelegate OnStartSkillDelegate;
	FPC_OnEndSkillDelegate OnEndSkillDelegate;
};

