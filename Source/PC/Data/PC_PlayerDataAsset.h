// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "PC/PC_Enum.h"
#include "Engine/DataAsset.h"
#include "PC_PlayerDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FPC_ActionStaminaData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPC_ActionType ActionType = EPC_ActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin ="0.0"))
	float StartCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin ="0.0"))
	float MaintainCostPerSec = 0.0f;
};

USTRUCT(BlueprintType)
struct FPC_SkillSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TMap<EPC_SkillSlotType, uint32> SkillIds;
};

USTRUCT(BlueprintType)
struct FPC_ComboKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EPC_CharacterStanceType CharacterStanceType = EPC_CharacterStanceType::Sword;

	UPROPERTY(EditAnywhere)
	bool bSpecialAction = false;

	FPC_ComboKey()
	{
	}

	FPC_ComboKey(const EPC_CharacterStanceType StanceType, bool SpecialAction)
	{
		CharacterStanceType = StanceType;
		bSpecialAction = SpecialAction;
	}

	bool operator==(const FPC_ComboKey& ComboAttackKey) const
	{
		return ComboAttackKey.CharacterStanceType == CharacterStanceType && ComboAttackKey.bSpecialAction ==
			bSpecialAction;
	}
};

USTRUCT(BlueprintType)
struct FPC_SkillEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FPC_ComboKey Key;

	UPROPERTY(EditAnywhere)
	FPC_SkillSlotData Data;
};

UCLASS()
class PC_API UPC_PlayerDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MovementSpeed_Walk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MovementSpeed_Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MovementSpeed_Jog;

	UPROPERTY(EditAnywhere)
	FName WeaponSocketName;

	UPROPERTY(EditAnywhere)
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> RollMontage;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> JumpSound;

	UPROPERTY(EditAnywhere)
	TArray<FPC_SkillEntry> SkillSlotDatas;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> BackstabMontage;

	//초당 얼만큼 스테미나를 회복 시킬지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina", meta=(ClampMin = "0.0"))
	float StaminaRegenPerSec = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina", meta=(ClampMin = "0.0"))
	float StaminaRegenDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina");
	TArray<FPC_ActionStaminaData> ActionStaminaDatas;
};
