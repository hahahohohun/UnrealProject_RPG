#pragma once

#include "CoreMinimal.h"
#include "PC_StatComponent.h"
#include "Components/ActorComponent.h"

#include "PC_StatusEffectComponent.generated.h"

USTRUCT()
struct FPC_StatusEffectInfo
{
	GENERATED_BODY()
public:	
	uint32 StatusEffectId;
	float RemainingTime;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FPC_OnStatusEffectTimeUpdate, uint32 StatusEffectId, float RemainingTime);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_StatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Called when the game starts
	UPC_StatusEffectComponent();
	
protected:
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	FPC_OnStatusEffectTimeUpdate OnStatusEffectTimeUpdate;
	
	UPROPERTY()
	TWeakObjectPtr<UPC_StatComponent> StatComponent;

	void ApplyStatusEffect(uint32 StatusEffectID);
	void RemoveEffect(uint32 StatusEffectID);

	UPROPERTY()
	TMap<uint32, UNiagaraComponent*> ActiveFXComponents;

	UPROPERTY()
	TMap<uint32, FPC_StatusEffectInfo> ActiveStatusEffectInfos;
};

