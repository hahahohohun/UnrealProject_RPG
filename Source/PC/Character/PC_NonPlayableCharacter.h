// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_BaseCharacter.h"
//#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterHUDInterface.h"
#include "PC_NonPlayableCharacter.generated.h"

UCLASS()
class PC_API APC_NonPlayableCharacter : public APC_BaseCharacter, public IPC_CharacterAIInterface
{
	GENERATED_BODY()
	

	
public:
	// Sets default values for this character's properties
	APC_NonPlayableCharacter();
protected:
	
	virtual void BeginPlay() override;
	virtual float GetAIPatrolRadius() override;
	virtual float GetAIDetectRange() override;
	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;

	virtual void SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;
	virtual void Attack() override;
	virtual void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);

private:
	FAICharacterAttackFinished OnAttackFinished;
	FPC_EnemyTableRow* EnemyTableRow = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> AttackAnim = nullptr;
};

