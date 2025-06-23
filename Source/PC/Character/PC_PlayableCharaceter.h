// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimMontage.h"
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "PC_BaseCharacter.h"
#include "Component/PC_SkillComponent.h"
#include "PC/Interface/PC_CharacterHUDInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC_PlayableCharaceter.generated.h"

class UPC_AimComponent;
class UPC_InputDataAsset;
class UPC_PlayerDataAsset;
class UPC_ActionComponent;
class UPC_LockOnComponent;
class UAIPerceptionStimuliSourceComponent;

UCLASS()
class PC_API APC_PlayableCharaceter : public APC_BaseCharacter , public IPC_CharacterHUDInterface, public IGenericTeamAgentInterface, public IPC_PlayerCharacterInterface
{
	GENERATED_BODY()
	

public:
	// Sets default values for this character's properties
	APC_PlayableCharaceter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);

	void Attack(const FInputActionValue& Value);
	void SpecialAction(const FInputActionValue& Value);
	void Run(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void WeaponSwap(const FInputActionValue& Value);
	void LockOn(const FInputActionValue& Value);

	void Num1(const FInputActionValue& Value);

public:

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupHUDWidget(UPC_HUDWidget* InWidget) override;

	void AdjustMovement(bool IsPressed);
	void AdjustCamera(bool bIsPressed);
	
	void SetGenericTeamId(const FGenericTeamId& TeamID);
	FGenericTeamId GetGenericTeamId() const;

	virtual UStaticMeshComponent* GetWeaponStaticMeshComponent() const override { return WeaponStaticMeshComponent; }
	virtual USpringArmComponent* GetSpringArmComponent() const override { return CameraBoom; }
	virtual UCameraComponent* GetCameraComponent() const override { return FollowCamera; }

	virtual UPC_ActionComponent* GetActionComponent() const override { return ActionComponent; }
	virtual UPC_LockOnComponent* GetLockOnComponent() const override { return LockOnComponent; }
	virtual UPC_BattleComponent* GetBattleComponent() const override { return BattleComponent; }
	virtual UPC_PlayerDataAsset* GetPlayerData() const override { return PlayerData; }

public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimulusSource;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_LockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_ActionComponent> ActionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_AimComponent> AimComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_SkillComponent> SkillComponent;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	
	//===============================================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_InputDataAsset> InputData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPC_PlayerDataAsset> PlayerData;
	
	UPROPERTY()
	FGenericTeamId GenericTeamId = 0;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ProjectileClass;
};
