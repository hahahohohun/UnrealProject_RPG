// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Component/PC_BattleComponent.h"
#include "Component/PC_StatusEffectComponent.h"
#include "GameFramework/Character.h"
#include "PC/PC_Enum.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"
#include "PC_BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterLocked, bool, bLocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackIndicatorChanged, bool, bLocked);


class UPC_WidgetComponent;
struct FPC_CharacterStatTableRow;
class UPC_StatComponent;
class UPC_BattleComponent;

UCLASS(config=Game)
class APC_BaseCharacter : public ACharacter, public IPC_CharacterInterface, public IPC_CharacterWidgetInterface
{
	GENERATED_BODY()

public:
	APC_BaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	virtual void AttackTrace(bool bStart, FName TraceStartBoneName, FName TraceEndBoneName) override;
	virtual void AttackTraceWithWeapon(bool bStart, bool bRight) override;
	virtual bool HasWeapon() override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;

public:
	void ApplyStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat);
	void ApplyStatusEffect(uint32 StatusEffectId, float RemainingTime);
	
	virtual void SetupCharacterWidget(class UPC_UserWidget* InUserWidget) override;
	virtual void SetupLockOnWidget(UPC_UserWidget* InUserWidget) override;
	virtual void SetupAttackIndicatorOnWidget(class UPC_UserWidget* InUserWidget) override;
	virtual void OnLocked(bool bLocked) override;
	virtual void LaunchCharacter(FVector StartPos, FVector CauserPos, float Power);
	virtual void OnAttackIndicator(bool bLocked) override;
	virtual void OnDead();
	
	virtual bool IsDead() override;

	virtual UStaticMeshComponent* GetWeapon_L_StaticMeshComponent() override { return Weapon_L_StaticComponent; }
	virtual UStaticMeshComponent* GetWeapon_R_StaticMeshComponent() override { return Weapon_R_StaticComponent; }
	
	virtual UPC_CrowdControlComponent* GetCrowdControlComponent() override { return CrowdControlComponent; }
	virtual TPair<FName, FName> GetWeaponTraceNames(bool bRight) override;
	virtual UPC_CharacterDataAsset* GetCharacterDataAsset() override { return CharacterData ;}

	virtual  FPC_OnStartSkillDelegate& GetOnStartSkillDelegate() override;
	virtual  FPC_OnEndSkillDelegate& GetOnEndSkillDelegate() override;
	
	virtual UPC_BattleComponent* GetBattleComponent() const override { return BattleComponent; }
	virtual UPC_SkillComponent* GetSkillComponent() const override { return SkillComponent; }
	virtual UPC_StatComponent* GetStatComponent() const override { return StatComponent; }
	
	
	UFUNCTION()
	virtual void OnStartCrowdControl(EPC_CrowdControlType CrowdType, AActor* actor);

	UFUNCTION()
	virtual void OnEndCrowdControl(EPC_CrowdControlType CrowdType, AActor* actor);


public:
	UPROPERTY(EditAnywhere)
	uint32 CharacterDataID = 0;

	FOnCharacterLocked OnCharacterLocked;
	FOnAttackIndicatorChanged OnAttackIndicatorChanged;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPC_BattleComponent> BattleComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta= (allowPrivateAccess=true))
	TObjectPtr<UPC_CrowdControlComponent> CrowdControlComponent;

	UPROPERTY()
	TObjectPtr<UPC_StatComponent> StatComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UPC_StatusEffectComponent> StatusEffectComponent = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPC_WidgetComponent> WidgetComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Weapon_L_StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Weapon_R_StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_SkillComponent> SkillComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Weapon_L_StaticComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Weapon_R_StaticComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_CharacterDataAsset> CharacterData;
};
