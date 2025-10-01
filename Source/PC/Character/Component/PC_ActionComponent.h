#pragma once


#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC/PC_Enum.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC_ActionComponent.generated.h"

USTRUCT(BlueprintType)
struct FPC_LockData
{
	GENERATED_BODY()

	UPROPERTY()
	EPC_LockCauseType LockCauseType = EPC_LockCauseType::None;

	UPROPERTY()
	EPC_ActionType LockType = EPC_ActionType::None;

	FPC_LockData()
	{
	}

	FPC_LockData(EPC_LockCauseType InCause, EPC_ActionType InType) : LockCauseType(InCause), LockType(InType)
	{
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PC_API UPC_ActionComponent : public UActorComponent
{
	GENERATED_BODY()
	UPC_ActionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void Tick_Running(float DeltaTime);

public:
	void Move(FVector2D MovementVector);
	void Jump(bool IsPressed);
	void Attack(bool IsPressed);
	void SpecialAction(bool bPressed);
	void Run(bool bPressed);
	void Roll(bool bPressed);
	void SwapWeapon(bool bPressed);
	void Backstab(bool bPressed); //뒷잡 암살

	bool CanAction(EPC_ActionType InActionType);

	bool IsLocked(EPC_ActionType InLockType);
	void AddLock(EPC_LockCauseType InLockCauseType, EPC_ActionType InLockType);
	void ReleaseLock(EPC_LockCauseType InLockCauseType, EPC_ActionType InLockType);
	void ForceReleaseLock(EPC_LockCauseType InLockCauseType);
	void ResetAllLock();

	void ProcessFreeMove();
	void ProcessLockOnMove();

	UFUNCTION()
	void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	void ComboAttackSave();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	void RotateToControlRotation();

	const FPC_ActionStaminaData* GetActionStaminaData(EPC_ActionType Type) const;
	bool TryConsumeStaminaOnActionStart(EPC_ActionType InActionType);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsInSpecialAction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsRolling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool SaveAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int AttackCount = 0;

	UPROPERTY(BlueprintReadOnly)
	FVector2D InputVector = FVector2D::ZeroVector;

	TArray<FPC_LockData> LockData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPC_ActionStaminaData> ActionStaminaDatas;
	
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	UAnimMontage* CurrentAttackMontage = nullptr;
};
