#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "PC/PC_Enum.h"
#include "PC_CrowdControlComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPC_OnStartCCDelegate, EPC_CrowdControlType, CrowdControlType, AActor*, Causer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPC_OnEndCCDelegate, EPC_CrowdControlType, CrowdControlType, AActor*, Causer);

struct FPC_CrowdControlInfo
{
	uint32 CrowdControlDataId = 0;
	EPC_CrowdControlType CrowdControlType = EPC_CrowdControlType::None;
	
	float LifeTime = 0.0f;
	float ElapsedTime = 0.0f;

	FVector CauserPos = FVector::ZeroVector;
	FRotator CauserRot = FRotator::ZeroRotator;

	FVector StartPos = FVector::ZeroVector;
	FRotator StartRot = FRotator::ZeroRotator;

	TObjectPtr<UNiagaraComponent> SpawnedFx = nullptr; //ex) cc걸렸을때 머리 뺑뺑 도는거
	TWeakObjectPtr<AActor> Causer = nullptr; //cc 유발자는 사라질 수 있으니 weak ptr

	bool bDurationCC = false;
	bool bValid = false;
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_CrowdControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Called when the game starts
	UPC_CrowdControlComponent();
	
protected:
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Tick_PlayCrowdControl(float DeltaTime);

	void ProcessCC(float DeltaTime);

	void RequestPlayerCC(uint32 CrowdControlId, AActor* Causer);
	bool CanPlayCC(FPC_CrowdControlInfo& info);
	void PlayCC(FPC_CrowdControlInfo& info);
	void StopCC();
	
	void OnStartCC();
	void OnStopCC();

	void PlayFX(FPC_CrowdControlInfo& Info);
	void StopFX();

	bool IsCrowdControlled();

	FPC_OnStartCCDelegate OnStartCCDelegate;
	FPC_OnEndCCDelegate OnEndCCDelegate;

	FPC_CrowdControlInfo CrowdControlInfo;
	
	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr; //내 소유가 아님
	
};

