// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PC_SeesawOscillator.generated.h"

UENUM(BlueprintType)
enum class EPC_OscAxis : uint8 { Roll_X, Pitch_Y, Yaw_Z };


UCLASS()
class PC_API APC_SeesawOscillator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APC_SeesawOscillator();
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Pivot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Platform;

	// === 조절 파라미터 ===
	UPROPERTY(EditAnywhere, Category="Osc")
	EPC_OscAxis Axis = EPC_OscAxis::Pitch_Y;     // 시소 느낌이면 Pitch_Y 권장

	UPROPERTY(EditAnywhere, Category="Osc", meta=(ClampMin="0.0", ClampMax="90.0"))
	float AmplitudeDeg = 12.f;                   // 최대 기울기(±도)

	UPROPERTY(EditAnywhere, Category="Osc", meta=(ClampMin="0.01", ClampMax="2.0"))
	float FrequencyHz = 0.25f;                   // 1초당 진동 수(느리게 0.2~0.5)

	UPROPERTY(EditAnywhere, Category="Osc")
	float PhaseDeg = 0.f;                        // 시작 위상(도)

	UPROPERTY(EditAnywhere, Category="Osc")
	TSubclassOf<UStaticMeshComponent> Platform2;

private:
	float TimeAcc = 0.f;
	FRotator InitialRelRot;
};
