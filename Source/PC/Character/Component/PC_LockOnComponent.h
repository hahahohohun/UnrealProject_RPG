// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC_LockOnComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_LockOnComponent : public UActorComponent
{
	GENERATED_BODY()


public:	
	// Sets default values for this component's properties
	UPC_LockOnComponent();
virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	void LockOn();

	APawn* FindTarget();
	void LockTarget(APawn* InActor);
	void ClearTarget();
	APawn* GetLockTarget() const { return LockedTarget; }

	virtual void SetLockOnMode(bool bEnable);
	bool IsLockOnMode() const { return bLockOnMode; }

private:
	UPROPERTY()
	TObjectPtr<APawn> LockedTarget = nullptr;

	float TargetDetectRadius = 0.f;
	float TargetDetectAngle = 0.f;

public:
	UPROPERTY(BlueprintReadOnly)
	bool bLockOnMode = false;
};
