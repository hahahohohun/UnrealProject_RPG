// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_ActionComponent.h"
#include "Components/ActorComponent.h"
#include "PC/Data//PC_TableRows.h"
#include "PC/Data/PC_CharacterDataAsset.h"
#include "PC_BattleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_BattleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPC_BattleComponent();
private:
	void Tick_Assassinate(float DeltaTime);
	void Tick_TraceWeapon(float DeltaTime);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	void StartTraceWithWeapon(bool bRight, bool bPowerAttack);
	void StartTrace(FName InTraceStartBoneName, FName InTraceEndBoneName);
	void EndTrace();

	void SwapWeapon();
	bool CanSwapWeapon();
	
	void EquipWeapon(uint8 InWeaponId, bool bRightHand);
	void UnEquipWeapon();
	bool HasWeapon();

	void FireProjectile(bool IsPressed);

	AActor* GetAssassinateTarget() const;
	bool TryAssassinate();
	void Assassinate(AActor* Target);

	bool bTracing = false;
	bool bTraceRightWeapon = false;
	bool bPowerAttack = false;
	
	FName TraceStartBoneName;
	FName TraceEndBoneName;

	FVector PrevStartBoneLocation = FVector::ZeroVector;
	FVector PrevEndBoneLocation = FVector::ZeroVector;

	float TraceInterval = 0.f;
	float TraceElapsedTime = 0.f;
	
	TArray<TWeakObjectPtr<AActor>> DamagedActor;
	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr;
	
	// Weapon ----------------------
	FPC_WeaponTableRow* Weapon_L_TableRow = nullptr;
	FPC_WeaponTableRow* Weapon_R_TableRow = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	EPC_CharacterStanceType CharacterStanceType;
	
	int32 CurWeaponIdx = 0;
	TArray<FPC_WeaponData> Weapons;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsAssassinated = false;

private:
	UPROPERTY()
	TObjectPtr<UPC_ActionComponent> ActionComponent;

	TWeakObjectPtr<ACharacter> AssassinateTarget;


	float AssassinatingElapsedTime = 0.f;
};


