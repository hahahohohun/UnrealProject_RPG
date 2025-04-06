// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Component/PC_BattleComponent.h"
#include "PC/Interface/PC_AnimationAttackInterface.h"
#include "PC_BaseCharacter.generated.h"


UCLASS(config=Game)
class APC_BaseCharacter : public ACharacter, public  IPC_AnimationAttackInterface
{
	GENERATED_BODY()

public:
	APC_BaseCharacter();
	

protected:
	
	virtual void AttackTrace(bool bStart, FName TraceBoneName) override;
protected:

	// To add mapping context
	virtual void BeginPlay();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	class UPC_BattleComponent* BattleComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	class UPC_StatComponent*  StatComponent = nullptr;
};

