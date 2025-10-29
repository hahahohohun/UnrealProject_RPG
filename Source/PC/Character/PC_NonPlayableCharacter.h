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
	virtual void SetDeadType(EPC_DeadType NewDeadType) override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;

	virtual void Tick(float DeltaTime) override;

	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;
	virtual FPC_EnemyTableRow* GetEnemyData() override;;

	virtual void SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;
	virtual void
	SetAIMoveMontageFinishedDelegate(const FAICharacterMoveMontageFinished& InOnMoveMontageFinished) override;
	virtual void Attack(bool bLastAttack) override;
	virtual void OnAttackMontageEnd(UAnimMontage* Montage, bool bInterrupted);
	virtual void OnDashBackMontageEnd(UAnimMontage* Montage, bool bInterrupted);

	virtual void SetAITurnFinishDelegate(const FAICharacterTurnFinished& InOnTurnFinished) override;
	virtual void TurnInPlace(float TurnAnimDegree) override;
	virtual void DashBack() override;
	virtual AActor* GetPatrolRoute() override;
	virtual void IncrementPatrolIndex() override;

	virtual void ResetState() override;
	virtual void RequestChangeState(EPC_EnemyStateType StateType) override;
	bool CanChangeState(EPC_EnemyStateType StateType);
	void ChangeState(EPC_EnemyStateType StateType);

	virtual void OnStartCrowdControl(EPC_CrowdControlType CrowdType, AActor* actor) override;
	virtual void OnEndCrowdControl(EPC_CrowdControlType CrowdType, AActor* actor) override;

	virtual void OnDead() override;
	virtual void SetupCharacterWidget(class UPC_UserWidget* InWidget) override;

	UFUNCTION(BlueprintCallable)
	virtual EPC_EnemyStateType GetState() override;

	UFUNCTION(BlueprintCallable)
	virtual EPC_DeadType GetDeadType() override;

	virtual void JumpToNextAttackMontage() override;
	virtual void ResetUsedMontage() override;
	virtual void ReactAttackBreak() override;
	virtual void OnStartSkill(uint32 SkillId) override;
	virtual void OnEndSkill(uint32 SkillId) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_WidgetComponent> IndicatorComponent;

	FAICharacterAttackFinished OnAttackFinished;
	FAICharacterTurnFinished OnTurnFinished;
	FAICharacterMoveMontageFinished OnMoveMontageFinished;

	FPC_EnemyTableRow* EnemyTableRow = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> DashBackAnimMontage = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> PatrolRoute = nullptr;

	UPROPERTY(EditAnywhere)
	bool HasSuperArmor = false; //CC공격 무시

	UPROPERTY(EditAnywhere)
	bool IsBossMonster = false;

	UPROPERTY(EditAnywhere)
	bool bHitPartInit = false; //

	UPROPERTY(EditAnywhere)
	FName Name = NAME_None;

	EPC_EnemyStateType EnemyState = EPC_EnemyStateType::None;

	EPC_DeadType DeadType = EPC_DeadType::None;

	bool IsTurning = false;

	float TurnStartYaw = 0.0f;
	float TurnDegree = 0.0f;

	TArray<TObjectPtr<UAnimMontage>> AlreadyPlayedAttackMontages;
	bool bLastAttacking; //마지막 공격
};
