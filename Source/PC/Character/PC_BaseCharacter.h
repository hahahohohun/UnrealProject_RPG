// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "PC/PC_Enum.h"
#include "PC/Interface/PC_AnimationAttackInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"
#include "PC_BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterLocked, bool, bLocked);

class UPC_WidgetComponent;
struct FPC_CharacterStatTableRow;
class UPC_StatComponent;
class UPC_BattleComponent;

UCLASS(config=Game)
class APC_BaseCharacter : public ACharacter, public IPC_AnimationAttackInterface , public IPC_CharacterWidgetInterface
{
	GENERATED_BODY()
	
public:
	APC_BaseCharacter();
protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	virtual void AttackTrace(bool bStart, FName TraceStartBoneName, FName TraceEndBoneName) override;
	virtual void AttackTraceWithWeapon(bool bStart) override;
	virtual bool HasWeapon() override;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
public:
	void ApplyStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat);

	virtual void SetupCharacterWidget(class UPC_UserWidget* InUserWidget) override;
	virtual void SetupLockOnWidget(UPC_UserWidget* InUserWidget) override;
	virtual void OnLocked(bool bLocked) override;
	
	
public:
	UPROPERTY(EditAnywhere)
	EPC_CharacterType CharacterType = EPC_CharacterType::None;
	
	FOnCharacterLocked OnCharacterLocked;
	
	UPROPERTY()
	TObjectPtr<UPC_BattleComponent> BattleComponent = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPC_StatComponent> StatComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UPC_WidgetComponent> WidgetComponent = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WeaponStaticMeshComponent;
};

