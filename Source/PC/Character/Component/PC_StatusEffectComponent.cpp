#include "PC_StatusEffectComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "PC/PC.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_StatusEffectComponent::UPC_StatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UPC_StatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	const IPC_CharacterInterface* Interface = CastChecked<IPC_CharacterInterface>(GetOwner());
	StatComponent = Interface->GetStatComponent();
	
	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UPC_StatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveStatusEffectInfos.Num() == 0 || !StatComponent.IsValid())
		return;
	
	// 안전한 삭제를 위해 이터레이터 사용
	for (auto It = ActiveStatusEffectInfos.CreateIterator(); It; ++It)
	{
		const uint32 EffectId = It.Key();
		FPC_StatusEffectInfo& Info = It.Value();

		if (Info.Duration <= KINDA_SMALL_NUMBER || Info.Type == EPC_StatusEffectType::None)
		{
			// 비정상 데이터 방어적 제거
			RemoveEffect(EffectId);
			continue;
		}

		const float ApplySeconds = FMath::Min(DeltaTime, Info.RemainingTime);
		const float RatePerSec = Info.ModifierValue / FMath::Max(Info.Duration, KINDA_SMALL_NUMBER);
		const float Amount = RatePerSec * ApplySeconds;

		ApplyStatus(Info.Type, Amount);

		Info.RemainingTime -= DeltaTime;
		OnStatusEffectTimeUpdate.Broadcast(EffectId, FMath::Max(Info.RemainingTime, 0.f));

		if (Info.RemainingTime <= 0.f)
		{
			RemoveEffect(EffectId);
		}
	}
}


void UPC_StatusEffectComponent::ApplyStatusEffect(uint32 StatusEffectID)
{
	UE_LOG(LogPC, Log, TEXT("StatusEffectID: %u"), StatusEffectID);

	FPC_StatusEffectTableRow* EffectTableRow = FPC_GameUtil::GetStatusEffectData(StatusEffectID);
	check(EffectTableRow);

	if(EffectTableRow->StatusType <= EPC_StatusEffectType::None)
		return;

	// 시작 FX (즉시 1회 재생)
	PlayStartFX(*EffectTableRow);
	
	// 즉시형
	if (EffectTableRow->EffectApplyType == EPC_StatusEffectApplyType::Instant)
	{
		ApplyStatus(EffectTableRow->StatusType, EffectTableRow->ModifierValue);

		// 즉시형도 종료 FX 재생
		PlayEndFX(*EffectTableRow);
		return;
	}

	// 지속형, 버프형 맵에 등록하여 Tick에서 비례 적용
	FPC_StatusEffectInfo* Found = ActiveStatusEffectInfos.Find(StatusEffectID);
	if (!Found)
	{
		FPC_StatusEffectInfo NewInfo;
		NewInfo.StatusEffectId = StatusEffectID;
		NewInfo.Duration = EffectTableRow->Duration;
		NewInfo.RemainingTime = EffectTableRow->Duration;
		NewInfo.ModifierValue = EffectTableRow->ModifierValue;
		NewInfo.Type = EffectTableRow->StatusType;

		//버프형
		if(EffectTableRow->EffectApplyType == EPC_StatusEffectApplyType::StatModifier)
		{
			NewInfo.bAppliedStatModifier = true;
			StatComponent->AddStatusEffect(StatusEffectID);
		}
		
		ActiveStatusEffectInfos.Add(StatusEffectID, NewInfo);

		// Cascade FX 시작
		StartCascadeFX(StatusEffectID, *EffectTableRow);
	}
	else
	{
		//갱신
		Found->Duration = EffectTableRow->Duration;
		Found->RemainingTime = EffectTableRow->Duration;
		Found->ModifierValue = EffectTableRow->ModifierValue;
		Found->Type = EffectTableRow->StatusType;

		// 이미 켜져 있으면 그대로 두고, 없으면 시작
		if (!ActiveFXComponents.Contains(StatusEffectID))
		{
			//버프형
			if(EffectTableRow->EffectApplyType == EPC_StatusEffectApplyType::StatModifier)
			{
				StatComponent->RemoveStatusEffect(StatusEffectID);
				StatComponent->AddStatusEffect(StatusEffectID);
			}
			
			StartCascadeFX(StatusEffectID, *EffectTableRow);
		}
	}
}

void UPC_StatusEffectComponent::RemoveEffect(uint32 StatusEffectID)
{
	FPC_StatusEffectInfo* Found = ActiveStatusEffectInfos.Find(StatusEffectID);
	
	if(Found && Found->bAppliedStatModifier && StatComponent.IsValid())
	{
		StatComponent->RemoveStatusEffect(StatusEffectID);
	}
	
	ActiveStatusEffectInfos.Remove(StatusEffectID);
	
	// 테이블 접근은 FX 종료용으로만 필요
	FPC_StatusEffectTableRow* Row = FPC_GameUtil::GetStatusEffectData(StatusEffectID);

	// Cascade FX 정리
	StopCascadeFX(StatusEffectID, Row);

	// 종료 FX 1회 재생
	if (Row)
	{
		PlayEndFX(*Row);
	}
}

FPC_StatusEffectInfo* UPC_StatusEffectComponent::GetActiveStatusEffectInfo(uint32 StatusEffectID)
{
	return ActiveStatusEffectInfos.Find(StatusEffectID);
}

void UPC_StatusEffectComponent::StartCascadeFX(uint32 StatusEffectID, const FPC_StatusEffectTableRow& Row)
{
	if (!OwnerCharacter.IsValid())
		return;

	if (IsValid(Row.EffectFx_Cascade))
	{
		USceneComponent* AttachComp = OwnerCharacter->GetCapsuleComponent();
		const FVector RelPos = Row.RelativePos_EffectFx_Cascade;
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			Row.EffectFx_Cascade,
			AttachComp,
			NAME_None,
			RelPos,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true);

		if (NiagaraComp)
		{
			ActiveFXComponents.Add(StatusEffectID, NiagaraComp);
		}
	}
}

void UPC_StatusEffectComponent::StopCascadeFX(uint32 StatusEffectID, const FPC_StatusEffectTableRow* RowPtr)
{
	if (UNiagaraComponent** FoundPtr = ActiveFXComponents.Find(StatusEffectID))
	{
		if (UNiagaraComponent* Comp = *FoundPtr)
		{
			UE_LOG(LogPC, Log, TEXT("[FX] StartCascade %u Comp=%p"), StatusEffectID, Comp);
			Comp->Deactivate();
			Comp->DestroyComponent();
		}
		
		ActiveFXComponents.Remove(StatusEffectID);
		UE_LOG(LogPC, Log, TEXT("[FX] StopCascade %u Found=%s"), StatusEffectID, *LexToString(ActiveFXComponents.Contains(StatusEffectID)));
	}
}

void UPC_StatusEffectComponent::PlayStartFX(const FPC_StatusEffectTableRow& Row)
{
	if (!OwnerCharacter.IsValid())
		return;

	if (IsValid(Row.EffectFX_Start))
	{
		const FVector SpawnLoc = OwnerCharacter->GetActorLocation();
		FPC_GameUtil::SpawnEffectAttached(Row.EffectFX_Start, OwnerCharacter->GetCapsuleComponent(), NAME_None,
					FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	}
}

void UPC_StatusEffectComponent::PlayEndFX(const FPC_StatusEffectTableRow& Row)
{
	if (!OwnerCharacter.IsValid())
		return;

	if (IsValid(Row.EffectFX_End))
	{
		const FVector SpawnLoc = OwnerCharacter->GetActorLocation();
		FPC_GameUtil::SpawnEffectAttached(Row.EffectFX_End, OwnerCharacter->GetCapsuleComponent(), NAME_None,
					FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	}
}

void UPC_StatusEffectComponent::ApplyStatus(EPC_StatusEffectType StatusType, float Amount)
{
	if(StatComponent.IsValid())
	{
		switch (StatusType)
		{
			case EPC_StatusEffectType::Heal:
				StatComponent->HealHp(Amount);
				break;
			case EPC_StatusEffectType::Stamina:
				StatComponent->AddStamina(Amount);
				break;
			default:
				break;
		}
	}
}
