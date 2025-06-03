#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardData.h"
#include "Engine/DataTable.h"
#include "PC/PC_Enum.h"
#include "PC_TableRows.generated.h"

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