#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"
#include "PC_NormalAttackDamageType.generated.h"

class UPC_NormalAttackDamageType;


// 2) DamageType 은 타입 구분용
UCLASS()
class PC_API UPC_NormalAttackDamageType : public UDamageType
{
	GENERATED_BODY()
};

USTRUCT()
struct FNormalAttackDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

	FNormalAttackDamageEvent()
		: FDamageEvent()
		, bPowerAttack(false)
	{
		DamageTypeClass = UPC_NormalAttackDamageType::StaticClass();
	}

	// 고유 ID
	static const int32 ClassID;

	virtual int32 GetTypeID() const override
	{
		return FNormalAttackDamageEvent::ClassID;
	}

	virtual bool IsOfType(int32 InId) const override
	{
		return InId == FNormalAttackDamageEvent::ClassID || FDamageEvent::IsOfType(InId);
	}

	bool bPowerAttack;
};


