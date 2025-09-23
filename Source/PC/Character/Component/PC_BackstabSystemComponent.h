// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC_BackstabSystemComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_BackstabSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPC_BackstabSystemComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual bool ExecuteBackstab();

public:
	void BackstabOn();

	void SetBackstabTarget(APawn* InActor);
	void ClearTarget();

	virtual void SetBackstabOnMode(bool bEnable);
	bool IsBackstabOnMode() const;

	TObjectPtr<APawn> GetBackstabTarget() const;

	float AttackRangeSqur = 0.0f ;
	
private:
	UPROPERTY()
	TObjectPtr<APawn> BackstabTarget = nullptr;

public:
	UPROPERTY(BlueprintReadOnly)
	bool bBackstabOnMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRange = 0.f;
};
