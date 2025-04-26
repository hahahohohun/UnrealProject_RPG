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
	virtual void PossessedBy(AController* NewController) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Tick(float DeltaTime) override;

	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;
	virtual FPC_EnemyTableRow* GetEnemyData() override;;

	virtual void SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;
	virtual void Attack() override;
	virtual void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);

	virtual void SetAITurnFinishDelegate(const FAICharacterTurnFinished& InOnTurnFinished) override;
	virtual void TurnInPlace(float TurnAnimDegree) override;

	virtual AActor* GetPatrolRoute() override;
	virtual void IncrementPatrolIndex() override;

	virtual void ResetState() override;
	virtual void ChangeState(EPC_EnemyStateType StateType) override;

	UFUNCTION(BlueprintCallable)
	virtual EPC_EnemyStateType GetState() override;

private:
	FAICharacterAttackFinished OnAttackFinished;
	FAICharacterTurnFinished OnTurnFinished;

	FPC_EnemyTableRow* EnemyTableRow = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> AttackAnim = nullptr;

	//Turn 상태
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TurnAnimMontage = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> PatrolRoute = nullptr;
	
	EPC_EnemyStateType EnemyState = EPC_EnemyStateType::None;
	
	bool IsTurning = false;

	float TurnStartYaw = 0.0f;
	float TurnDegree = 0.0f;
};
