#pragma once
#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "NiagaraSystem.h"
#include "BehaviorTree/BlackboardData.h"
#include "Engine/DataTable.h"

#include "PC/PC_Enum.h"
#include "PC_TableRows.generated.h"

struct FPC_HitPartUnitAttackAnims;
class ALevelSequenceActor;
//에디터에서 셋팅하는 데이터
USTRUCT(BlueprintType)
struct FPC_ExecData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float DelayTime = 0.0f;

	UPROPERTY(EditAnywhere)
	uint32 ExecDataId = 0;
};

USTRUCT(BlueprintType)
struct FPC_HitPartData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName HitPartName = NAME_None;

	UPROPERTY(EditAnywhere)
	EPC_HitPartType HitPartType = EPC_HitPartType::None;

	UPROPERTY(EditAnywhere)
	float AddHitDamage = 0.0f; // 부위 별 추가뎀지

	UPROPERTY(EditAnywhere)
	UMaterialInterface* HitPartMaterial = nullptr;
};

USTRUCT(BlueprintType)
struct FPC_HitPartUnitAttackAnims
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UAnimMontage>> AnimsMontages;
};


USTRUCT(BlueprintType)
struct FPC_CharacterStatTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FPC_CharacterStatTableRow() : MaxHp(0.0f), MaxStamina(0.0f), Attack(0.0f), PowerAttack(0.0f), MovementSpeed(0.0f)
	{
	}

	UPROPERTY(EditAnywhere)
	uint32 CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float MaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float PowerAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float MaxStamina;

	FPC_CharacterStatTableRow operator+(const FPC_CharacterStatTableRow& Other) const
	{
		FPC_CharacterStatTableRow Result;

		Result.MaxHp = this->MaxHp + Other.MaxHp;
		Result.Attack = this->Attack + Other.Attack;
		Result.PowerAttack = this->PowerAttack + Other.PowerAttack;
		Result.MaxStamina = this->MaxStamina + Other.MaxStamina;
		Result.MovementSpeed = this->MovementSpeed + Other.MovementSpeed;

		return Result;
	}
};


USTRUCT(BlueprintType)
struct FPC_EnemyTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Type)
	int32 EnemyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USkeletalMesh*> SkeletalMeshs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBlackboardData> BlackBoard = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed_Walk = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed_Run = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed_Strafe = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurnSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SightRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoseSightRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SightAngle = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRange = 0.f;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UAnimMontage>> AttackAnims;

	UPROPERTY(EditAnywhere)
	TMap<EPC_ProximityType, FPC_HitPartUnitAttackAnims> HitPartAttackAnims;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> DashBackAnim = nullptr;


	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> AttackBreakAnim = nullptr; //공격이 막혔을때 ex)플레이어 Guard중 

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Left90TurnAnim = nullptr;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Right90TurnAnim = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Left45TurnAnim = nullptr;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Right45TurnAnim = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Turn180Anim = nullptr;


	UPROPERTY(EditAnywhere)
	bool IsBoss = false;

	UPROPERTY(EditAnywhere)
	bool IsHitPartUnit = false;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> HitPartFX = nullptr;

	UPROPERTY(EditAnywhere)
	bool HasSuperAmor = false;

	UPROPERTY(EditAnywhere)
	FName Name = NAME_None;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<ULevelSequence> DeathSequenceAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> SpawnItem;
};


USTRUCT(BlueprintType)
struct FPC_WeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 WeaponId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> WeaponMesh = nullptr;

	UPROPERTY(EditAnywhere)
	USoundBase* HitSound = nullptr;

	UPROPERTY(EditAnywhere)
	USoundBase* SwingSound = nullptr;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* WeaponSparkFX_Niagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TraceStartSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TraceEndSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RelativePos = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RelativeRot = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RelativeScale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct FPC_SkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint32 DataId = 0;

	UPROPERTY(EditAnywhere)
	float CoolTime = 0.f;

	UPROPERTY(EditAnywhere)
	float SkillRange = 0.f;

	UPROPERTY(EditAnywhere)
	EPC_SkillTargetingType SkillTargetingType = EPC_SkillTargetingType::None;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* SkillActiveFx;

	UPROPERTY(EditAnywhere)
	TArray<FPC_ExecData> ExecDatas;
};

USTRUCT(BlueprintType)
struct FPC_ExecTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint32 DataId = 0;

	UPROPERTY(EditAnywhere, Category="SFX")
	TObjectPtr<USoundBase> StartSFX; //스킬 시작할때

	UPROPERTY(EditAnywhere, Category="SFX")
	TObjectPtr<USoundBase> ActiveSFX; //

	UPROPERTY(EditAnywhere, Category="Decal")
	TObjectPtr<UAnimMontage> SkillAnim;

	UPROPERTY(EditAnywhere, Category="Decal")
	TObjectPtr<UMaterialInterface> SkillDecalMaterial;

	UPROPERTY(EditAnywhere, Category="Decal")
	FVector DecalSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Decal")
	FVector DecalRelativePos = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Decal")
	FRotator DecalRelativeRot = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category="Effect")
	UNiagaraSystem* ExecFX_Niagara_Start = nullptr;

	UPROPERTY(EditAnywhere, Category="Effect")
	UParticleSystem* ExecFX_Cascade_Start = nullptr;

	UPROPERTY(EditAnywhere, Category="Effect")
	UNiagaraSystem* ExecFX_Niagara_End = nullptr;

	UPROPERTY(EditAnywhere, Category="Effect")
	UParticleSystem* ExecFX_Cascade_End = nullptr;

	//받았을때 데미지 이펙트
	UPROPERTY(EditAnywhere, Category="Effect")
	UNiagaraSystem* HitFX_Niagara = nullptr;

	UPROPERTY(EditAnywhere, Category="Effect")
	UParticleSystem* HitFX_Cascade = nullptr;
	UPROPERTY(EditAnywhere, Category="Effect")
	float HitEffectScale = 1.f;
	UPROPERTY(EditAnywhere, Category="Effect")
	EPC_SkillFxAttachType SkillFxAttachType = EPC_SkillFxAttachType::None;

	//

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> ExeCurve;

	UPROPERTY(EditAnywhere)
	float Duration = 0.f;

	UPROPERTY(EditAnywhere)
	float Damage = 0.f;

	UPROPERTY(EditAnywhere)
	FRotator ProjectileAdditiveRot = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	FVector ProjectileAdditivePos = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	EPC_ExecType ExecType = EPC_ExecType::None;

	UPROPERTY(EditAnywhere)
	EPC_ExecCollisionType ExecCollisionType = EPC_ExecCollisionType::None;

	//EPC_ExecType에 따라 유동적으로 값에 의미가 바뀜
	UPROPERTY(EditAnywhere)
	float ExecProperty_0 = 0;

	UPROPERTY(EditAnywhere)
	float ExecProperty_1 = 0;

	UPROPERTY(EditAnywhere)
	float ExecProperty_2 = 0;
	//

	UPROPERTY(EditAnywhere, Category="Coliison")
	bool bSpawnCollision = true;

	//콜리전 정보 ex) 높이 너비 길이
	UPROPERTY(EditAnywhere, Category="Coliison")
	float ExecCollisionProperty_0 = 0;

	UPROPERTY(EditAnywhere, Category="Coliison")
	float ExecCollisionProperty_1 = 0;

	UPROPERTY(EditAnywhere, Category="Coliison")
	float ExecCollisionProperty_2 = 0;
	
	UPROPERTY(EditAnywhere, Category="Coliison")
	FRotator ExecCollisionRelativeRot = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	float CrowdControlId = INDEX_NONE;

	UPROPERTY(EditAnywhere)
	FName SkillPosBoneName = NAME_None;


	UPROPERTY(EditAnywhere, Category="ETC")
	bool bEffectBlur = false;

	UPROPERTY(EditAnywhere, Category="ETC")
	float HitDilationTime = 0.0f;

	UPROPERTY(EditAnywhere, Category="Material")
	TObjectPtr<UMaterialInterface> MaterialInterface = nullptr;

	UPROPERTY(EditAnywhere, Category="Material")
	bool bPlayHitMaterial = true;

	UPROPERTY(EditAnywhere, Category="Camera")
	EPC_CameraShakeMagnitudeType ShakeMagnitude = EPC_CameraShakeMagnitudeType::Weak;

	UPROPERTY(EditAnywhere, Category="Camera")
	EPC_CameraShakeActionType CameraShakeAction = EPC_CameraShakeActionType::None;


	UPROPERTY(EditAnywhere, Category="CineCamera")
	uint32 LinkSkillId = 0;

	UPROPERTY(EditAnywhere, Category="CineCamera")
	ULevelSequence* AttackSequenceAsset;

	UPROPERTY(EditAnywhere, Category="AfterImage")
	TSubclassOf<AActor> AfterImageActorClass;

	UPROPERTY(EditAnywhere, Category="AfterImage") //몇초간격으로 스폰시킬지
	float PosableMeshSpawnInterval = 0.f;
	

};

USTRUCT(BlueprintType)
struct FPC_SkillObjectTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool IsCollisionDestroy = true;

	UPROPERTY(EditAnywhere)
	uint32 DataId = 0;

	UPROPERTY(EditAnywhere)
	float Damage = 0;

	UPROPERTY(EditAnywhere)
	EPC_SkillObjectType SkillObjectType = EPC_SkillObjectType::None;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> SkillObjectActor;
};

USTRUCT()
struct FPC_CrowdControlTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint32 DataId = 0;

	UPROPERTY(EditAnywhere)
	EPC_CrowdControlType CrowdControlType = EPC_CrowdControlType::None;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> CrowdControlAnim = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> CrowdControlFX = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> EndCrowdControlFX = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInstance> MaterialInstance = nullptr; //메테리얼중 가장 상위 클래스

	UPROPERTY(EditAnywhere)
	float Duration = 0.f;

	UPROPERTY(EditAnywhere)
	float Property_0 = 0.f;
	UPROPERTY(EditAnywhere)
	float Property_1 = 0.f;
	UPROPERTY(EditAnywhere)
	float Property_2 = 0.f;
};

USTRUCT()
struct FPC_StatusEffectTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint32 DataId = 0;

	UPROPERTY(EditAnywhere)
	EPC_StatusEffectApplyType EffectApplyType = EPC_StatusEffectApplyType::Instant;

	UPROPERTY(EditAnywhere)
	EPC_StatusEffectType StatusType = EPC_StatusEffectType::None;

	UPROPERTY(EditAnywhere)
	float ModifierValue = 0;

	UPROPERTY(EditAnywhere)
	EPC_ValueMode ValueMode = EPC_ValueMode::Additive;

	UPROPERTY(EditAnywhere)
	float Duration = 0;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> Icon; // UI 아이콘

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> EffectFX_Start = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> EffectFX_End = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> EffectFx_Cascade = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RelativePos_EffectFx_Cascade = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FPC_AnimMontageRootMotionDistanceRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	FSoftObjectPath MontagePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	float Distance = 0.f;
};


USTRUCT(BlueprintType)
struct FPC_PhysicSkeletonBoneNameRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	FSoftObjectPath PhysicAssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	float Distance = 0.f;
};

USTRUCT(BlueprintType)
struct FPC_HitPartListRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftObjectPath PhysicsAssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPC_HitPartData> HitPartDatas;
};
