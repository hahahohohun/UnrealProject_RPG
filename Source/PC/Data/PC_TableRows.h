#pragma once
#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "BehaviorTree/BlackboardData.h"
#include "Engine/DataTable.h"
#include "PC/PC_Enum.h"
#include "PC_TableRows.generated.h"

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
struct FPC_CharacterStatTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	public:
	FPC_CharacterStatTableRow() :MaxHp(0.0f), Attack(0.0f), MovementSpeed(0.0f){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	EPC_CharacterType CharacterType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float MaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float MovementSpeed;

	FPC_CharacterStatTableRow operator+(const FPC_CharacterStatTableRow& Other) const
	{
		FPC_CharacterStatTableRow Result;

		Result.MaxHp = this->MaxHp + Other.MaxHp;
		Result.Attack = this->Attack + Other.Attack;
		Result.MovementSpeed = this->MovementSpeed + Other.MovementSpeed;
		
		return Result;
	}
	
};

USTRUCT(BlueprintType)
struct FPC_EnemyTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Type)
	EPC_CharacterType EnemyType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh = nullptr;

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
	TObjectPtr<UAnimMontage> AttackAnim = nullptr;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Left90TurnAnim = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Left180TurnAnim = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Right90TurnAnim = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Right180TurnAnim = nullptr;
};


USTRUCT(BlueprintType)
struct FPC_WeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 WeaponId = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> WeaponMesh = nullptr;

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

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> SkillAnim;

	UPROPERTY(EditAnywhere)
	float Duration = 0.f;

	UPROPERTY(EditAnywhere)
	EPC_ExecType ExecType = EPC_ExecType::None;

	//EPC_ExecType에 따라 유동적으로 값에 의미가 바뀜
	UPROPERTY(EditAnywhere)
	float ExecProperty_0 = 0;

	UPROPERTY(EditAnywhere)
	float ExecProperty_1 = 0;

	UPROPERTY(EditAnywhere)
	float ExecProperty_2 = 0;
	//
	
};

USTRUCT(BlueprintType)
struct FPC_SkillObjectTableRow : public  FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint32 DataId = 0;
	
	UPROPERTY(EditAnywhere)
	EPC_SkillObjectType SkillObjectType = EPC_SkillObjectType::None;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> SkillObjectActor;
};
