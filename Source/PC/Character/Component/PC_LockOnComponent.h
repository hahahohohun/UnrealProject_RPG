// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC_LockOnComponent.generated.h"

class USpringArmComponent;

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
	APawn* GetLockTarget() const { return LockedTarget.Get(); }

	virtual void SetLockOnMode(bool bEnable);
	bool IsLockOnMode() const { return bLockOnMode; }

protected:
	// 추가: 보스 크기 기반 카메라 거리 계산용
	FVector GetLockOnViewPoint(AActor* TargetActor);
	float GetTargetHeight(AActor* TargetActor) const;
	float GetDesiredArmLength(AActor* TargetActor) const;
	void UpdateCameraArmLength(float DeltaTime);

public:
	UPROPERTY(BlueprintReadOnly)
	bool bLockOnMode = false;
	
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	TWeakObjectPtr<APawn> LockedTarget;

	USpringArmComponent* SpringArm;
	FVector TargetOverViewPoint;
	float TargetDetectRadius = 0.f;
	float TargetDetectAngle = 0.f;

	// 카메라 암 기본 값 캐싱
	float DefaultArmLength = 0.f;

	// 최소/최대 거리
	UPROPERTY(EditAnywhere, Category="LockOn|Camera")
	float MinArmLength = 300.f;

	UPROPERTY(EditAnywhere, Category="LockOn|Camera")
	float MaxArmLength = 400.f;

	// 보간 속도
	UPROPERTY(EditAnywhere, Category="LockOn|Camera")
	float ArmInterpSpeed = 5.f;

	bool bCachedDefaultArmLength = false;
	
};
