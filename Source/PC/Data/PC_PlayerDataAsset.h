// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PC_PlayerDataAsset.generated.h"


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

	FPC_ComboKey(){}

	FPC_ComboKey(const EPC_CharacterStanceType StanceType, bool SpecialAction)
	{
		CharacterStanceType = StanceType;
		bSpecialAction = SpecialAction;
	}

	bool operator==(const FPC_ComboKey& ComboAttackKey) const
	{
		return ComboAttackKey.CharacterStanceType == CharacterStanceType && ComboAttackKey.bSpecialAction == bSpecialAction;
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
};
