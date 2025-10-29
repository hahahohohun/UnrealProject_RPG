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
	UPROPERTY()
	uint32 StatusEffectId = 0;

	UPROPERTY()
	float RemainingTime = 0.f;

	UPROPERTY()
	float Duration = 0.f;

	UPROPERTY()
	float ModifierValue = 0.f;

	UPROPERTY()
	EPC_StatusEffectType Type = EPC_StatusEffectType::None;

	bool bAppliedStatModifier = false;
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
	FPC_OnStatusEffectTimeUpdate OnStatusEffectTimeUpdate;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void ApplyStatusEffect(uint32 StatusEffectID);
	UFUNCTION()
	void RemoveEffect(uint32 StatusEffectID);
	
	FPC_StatusEffectInfo* GetActiveStatusEffectInfo(uint32 StatusEffectID);
	
	UPROPERTY()
	TWeakObjectPtr<UPC_StatComponent> StatComponent;

private:
	UPROPERTY()
	TMap<uint32, UNiagaraComponent*> ActiveFXComponents;

	UPROPERTY()
	TMap<uint32, FPC_StatusEffectInfo> ActiveStatusEffectInfos;

	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr;

	void StartCascadeFX(uint32 StatusEffectID, const FPC_StatusEffectTableRow& Row);
	void StopCascadeFX(uint32 StatusEffectID, const FPC_StatusEffectTableRow* RowPtr);
	void PlayStartFX(const FPC_StatusEffectTableRow& Row);
	void PlayEndFX(const FPC_StatusEffectTableRow& Row);
	void ApplyStatus(EPC_StatusEffectType Type, float Amount);
};

