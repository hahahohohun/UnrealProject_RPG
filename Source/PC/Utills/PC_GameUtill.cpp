#include "PC_GameUtill.h"

#include "NiagaraFunctionLibrary.h"
#include "Logging/LogMacros.h"
#include  "CoreMinimal.h"
#include "AIController.h"
#include "PC/PC.h"
#include "PC/Cometic/PC_LegacyCameraShake.h"
#include "CoreTypes.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "DSP/MidiNoteQuantizer.h"
#include "DynamicMesh/MeshTransforms.h"
#include "Sound/SoundWave.h"
#include "GameFramework/Character.h"
#include "PC/Data/PC_CharacterDataAsset.h"
#include "PC/Data/PC_HitPartDataAsset.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/Misc/GameMode/PCGameMode.h"
#include "PC/Subsystem/PC_AudioSubsystem.h"
#include "PC/Subsystem/PC_UISubsystem.h"


FPC_CharacterStatTableRow* FPC_GameUtil::GetCharacterStatData(uint32 CharacterId)
{
	TArray<FPC_CharacterStatTableRow*> CharacterTableRows = GetAllRows<FPC_CharacterStatTableRow>(
		EPC_DataTableType::CharacterStat);
	if (FPC_CharacterStatTableRow** FoundRow = CharacterTableRows.FindByPredicate(
		[CharacterId](const FPC_CharacterStatTableRow* Row)
		{
			return Row->CharacterId == CharacterId;
		}))
	{
		return *FoundRow;
	}

	return nullptr;
}

FPC_EnemyTableRow* FPC_GameUtil::GetEnemyData(uint32 CharacterType)
{
	TArray<FPC_EnemyTableRow*> CharacterTableRows = GetAllRows<FPC_EnemyTableRow>(EPC_DataTableType::Enemy);
	if (FPC_EnemyTableRow** FoundRow = CharacterTableRows.FindByPredicate([CharacterType](const FPC_EnemyTableRow* Row)
	{
		return Row->EnemyType == CharacterType;
	}))
	{
		return *FoundRow;
	}

	return nullptr;
}

FPC_WeaponTableRow* FPC_GameUtil::GetWeaponData(uint32 WeaponId)
{
	TArray<FPC_WeaponTableRow*> EnemyTableRows = GetAllRows<FPC_WeaponTableRow>(EPC_DataTableType::Weapon);
	if (FPC_WeaponTableRow** FoundRow = EnemyTableRows.FindByPredicate([WeaponId](const FPC_WeaponTableRow* Row)
	{
		return Row->WeaponId == WeaponId;
	}))
	{
		return *FoundRow;
	}

	UE_LOG(LogPC, Error, TEXT("WeaponData is Invalid"));
	return nullptr;
}

UPC_CameraDataAsset* FPC_GameUtil::GetCameraData(EPC_CameraType CameraType)
{
	if (GEngine)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_DataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UPC_DataSubsystem>())
			{
				return *DataSubsystem->CameraData.Find(CameraType);
			}
		}
	}

	return nullptr;
}

UPC_GameDataAsset* FPC_GameUtil::GetGameData()
{
	if (GEngine)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_DataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UPC_DataSubsystem>())
			{
				return DataSubsystem->GameDataAsset;
			}
		}
	}

	return nullptr;
}

FPC_SkillTableRow* FPC_GameUtil::GetSkillData(uint32 Skillid)
{
	TArray<FPC_SkillTableRow*> EnemyTableRows = GetAllRows<FPC_SkillTableRow>(EPC_DataTableType::Skill);
	if (FPC_SkillTableRow** FoundRow = EnemyTableRows.FindByPredicate([Skillid](const FPC_SkillTableRow* Row)
	{
		return Row->DataId == Skillid;
	}))
	{
		return *FoundRow;
	}

	UE_LOG(LogPC, Error, TEXT("Skill data is Invalid"));
	return nullptr;
}

FPC_SkillObjectTableRow* FPC_GameUtil::GetSkillObjectData(uint32 Skillid)
{
	TArray<FPC_SkillObjectTableRow*> EnemyTableRows = GetAllRows<FPC_SkillObjectTableRow>(
		EPC_DataTableType::SkillObject);
	if (FPC_SkillObjectTableRow** FoundRow = EnemyTableRows.FindByPredicate(
		[Skillid](const FPC_SkillObjectTableRow* Row)
		{
			return Row->DataId == Skillid;
		}))
	{
		return *FoundRow;
	}

	UE_LOG(LogPC, Error, TEXT("skill object data is Invalid"));
	return nullptr;
}

FPC_ExecTableRow* FPC_GameUtil::GetExecData(uint32 Uint32)
{
	TArray<FPC_ExecTableRow*> EnemyTableRows = GetAllRows<FPC_ExecTableRow>(EPC_DataTableType::Exec);
	if (FPC_ExecTableRow** FoundRow = EnemyTableRows.FindByPredicate([Uint32](const FPC_ExecTableRow* Row)
	{
		return Row->DataId == Uint32;
	}))
	{
		return *FoundRow;
	}

	UE_LOG(LogPC, Error, TEXT("ExecData data is Invalid"));
	return nullptr;
}

FPC_CrowdControlTableRow* FPC_GameUtil::GetCrowdControlData(uint32 crowdId)
{
	TArray<FPC_CrowdControlTableRow*> EnemyTableRows = GetAllRows<FPC_CrowdControlTableRow>(
		EPC_DataTableType::CrowdControl);
	if (FPC_CrowdControlTableRow** FoundRow = EnemyTableRows.FindByPredicate(
		[crowdId](const FPC_CrowdControlTableRow* Row)
		{
			return Row->DataId == crowdId;
		}))
	{
		return *FoundRow;
	}

	UE_LOG(LogPC, Error, TEXT("crowdId object data is Invalid"));
	return nullptr;
}

FPC_StatusEffectTableRow* FPC_GameUtil::GetStatusEffectData(uint32 statusEffectId)
{
	TArray<FPC_StatusEffectTableRow*> TableRows = GetAllRows<FPC_StatusEffectTableRow>(
		EPC_DataTableType::StatusEffect);
	if (FPC_StatusEffectTableRow** FoundRow = TableRows.FindByPredicate(
		[statusEffectId](const FPC_StatusEffectTableRow* Row)
		{
			return Row->DataId == statusEffectId;
		}))
	{
		return *FoundRow;
	}

	UE_LOG(LogPC, Error, TEXT("statusEffect object data is Invalid"));
	return nullptr;
}

float FPC_GameUtil::GetRootMotionDistanceData(FSoftObjectPath& ObjectPath)
{
	TArray<FPC_AnimMontageRootMotionDistanceRow*> RootMotionDistanceRows = GetAllRows<
		FPC_AnimMontageRootMotionDistanceRow>(EPC_DataTableType::RootMotionDistance);

	if (FPC_AnimMontageRootMotionDistanceRow** FoundRow = RootMotionDistanceRows.FindByPredicate([&ObjectPath](
		const FPC_AnimMontageRootMotionDistanceRow* Row)
		{
			return Row->MontagePath == ObjectPath;
		}))
	{
		return (*FoundRow)->Distance;
	}

	UE_LOG(LogPC, Error, TEXT("RootMotionDistance object data is Invalid"));
	return 0.f;
}

float FPC_GameUtil::CalculateRootMotionDistance(UAnimMontage* AnimMontage)
{
	if (!AnimMontage)
	{
		return 0.f;
	}

	float TotalDistance = 0.f;

	for (const FSlotAnimationTrack& SlotTrack : AnimMontage->SlotAnimTracks)
	{
		for (const FAnimSegment& Segment : SlotTrack.AnimTrack.AnimSegments)
		{
			if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(Segment.AnimReference))
			{
				if (!AnimSequence->bEnableRootMotion)
				{
					continue;
				}

				const float SequenceLength = AnimSequence->GetPlayLength();
				const float DeltaTime = 0.01f;

				float CurrentTime = 0.01f;
				FTransform CurrentAccumulatedTransform = FTransform::Identity;

				while (CurrentTime < SequenceLength)
				{
					float NextTime = FMath::Min(CurrentTime + DeltaTime, SequenceLength);

					const FTransform SegmentDeltaTransform = AnimSequence->ExtractRootMotionFromRange(
						CurrentTime, NextTime);
					CurrentAccumulatedTransform.Accumulate(SegmentDeltaTransform);

					CurrentTime = NextTime;
				}

				TotalDistance += CurrentAccumulatedTransform.GetTranslation().Size();
			}
		}
	}

	return TotalDistance;
}

FPC_HitPartListRow* FPC_GameUtil::GetHitPartData(FSoftObjectPath& ObjectPath)
{
	TArray<FPC_HitPartListRow*> HitRows = GetAllRows<FPC_HitPartListRow>(EPC_DataTableType::HitPart);
	if (FPC_HitPartListRow** FoundRow = HitRows.FindByPredicate([&ObjectPath](const FPC_HitPartListRow* Row)
	{
		return Row->PhysicsAssetPath == ObjectPath;
	}))
	{
		return *FoundRow;
	}

	UE_LOG(LogPC, Error, TEXT("HitPartData is Invalid"));
	return nullptr;
}

EPC_HitPartType FPC_GameUtil::GetHitPartTypeByName(FName BoneName, UDataAsset* DataAsset)
{
	UPC_HitPartDataAsset* HitPartDataAsset = Cast<UPC_HitPartDataAsset>(DataAsset);
	if (!HitPartDataAsset)
		return EPC_HitPartType::None;

	for (FString KeyWord : HitPartDataAsset->HeadKeywords)
	{
		if (BoneName.ToString().Contains(KeyWord))
		{
			return EPC_HitPartType::Head;
		}
	}

	for (FString KeyWord : HitPartDataAsset->BodyKeywords)
	{
		if (BoneName.ToString().Contains(KeyWord))
		{
			return EPC_HitPartType::Body;
		}
	}

	for (FString KeyWord : HitPartDataAsset->ArmKeywords)
	{
		if (BoneName.ToString().Contains(KeyWord))
		{
			for (FString Marker : HitPartDataAsset->LeftMarkers)
			{
				if (BoneName.ToString().Contains(Marker))
				{
					return EPC_HitPartType::Arm_l;
				}
			}

			for (FString Marker : HitPartDataAsset->RightMarkers)
			{
				if (BoneName.ToString().Contains(Marker))
				{
					return EPC_HitPartType::Arm_r;
				}
			}
		}
	}

	for (FString KeyWord : HitPartDataAsset->LegKeywords)
	{
		if (BoneName.ToString().Contains(KeyWord))
		{
			for (FString Marker : HitPartDataAsset->LeftMarkers)
			{
				if (BoneName.ToString().Contains(Marker))
				{
					return EPC_HitPartType::Leg_l;
				}
			}

			for (FString Marker : HitPartDataAsset->RightMarkers)
			{
				if (BoneName.ToString().Contains(Marker))
				{
					return EPC_HitPartType::Leg_r;
				}
			}
		}
	}

	return EPC_HitPartType::None;
}

UAnimMontage* FPC_GameUtil::GetProperAttackMontage(TArray<TObjectPtr<UAnimMontage>>& AlreadyPlayedMontage,
                                                   AActor* AttackActor, FVector TargetPos)
{
	IPC_CharacterAIInterface* CharacterAIInterface = Cast<IPC_CharacterAIInterface>(AttackActor);
	if (!CharacterAIInterface)
		return nullptr;

	FPC_EnemyTableRow* EnemyTableRow = CharacterAIInterface->GetEnemyData();
	if (!EnemyTableRow)
		return nullptr;

	bool IsHitPartUnit = EnemyTableRow->IsHitPartUnit;

	TArray<TObjectPtr<UAnimMontage>>& AnimMontages = EnemyTableRow->AttackAnims;
	//TMap<EPC_ProximityType, FPC_HitPartUnitAttackAnims>& HitPartAnimMontages = EnemyTableRow->IsHitPartUnit;

	auto BuildCandidates = [&]()
	{
		TArray<UAnimMontage*> Out;
		Out.Reserve(AnimMontages.Num());

		for (UAnimMontage* Montage : AnimMontages)
		{
			if (Montage && !AlreadyPlayedMontage.Contains(Montage))
				Out.Add(Montage);
		}

		return Out;
	};

	TArray<UAnimMontage*> Candidates = BuildCandidates();

	if (Candidates.Num() == 0)
	{
		if (AlreadyPlayedMontage.Num() > 0)
		{
			UAnimMontage* Last = AlreadyPlayedMontage.Last();
			AlreadyPlayedMontage.Reset();
			AlreadyPlayedMontage.Add(Last);
		}

		Candidates = BuildCandidates();

		if (Candidates.Num() == 0)
		{
			if (AnimMontages.Num() == 1 && AnimMontages[0])
				return AnimMontages[0];

			if (!IsHitPartUnit)
				return nullptr;
		}
	}

	ACharacter* Character = Cast<ACharacter>(AttackActor);
	if (!Character)
		return nullptr;

	AAIController* Controller = Cast<AAIController>(Character->GetController());
	if (!Controller)
		return nullptr;

	FVector CurrentPos = AttackActor->GetActorLocation();
	float DistFromTarget = FVector::Dist(CurrentPos, TargetPos);

	TArray<TPair<UAnimMontage*, float>> MontageInfos;
	MontageInfos.Reserve(Candidates.Num());

	UAnimMontage* ProperMontage = nullptr;

	if (IsHitPartUnit)
	{
		const EPC_ProximityType TargetProximity = static_cast<EPC_ProximityType>(Controller->GetBlackboardComponent()->
			GetValueAsEnum(TEXT("TargetProximityType")));

		if (FPC_HitPartUnitAttackAnims* HitPartUnitAttackAnims = EnemyTableRow->HitPartAttackAnims.
			Find(TargetProximity))
		{
			int32 ArrayNum = HitPartUnitAttackAnims->AnimsMontages.Num();
			const int32 PickIdx = FMath::RandRange(0, ArrayNum - 1);

			ProperMontage = HitPartUnitAttackAnims->AnimsMontages[PickIdx];
		}
	}
	else
	{
		for (UAnimMontage* AnimMontage : Candidates)
		{
			FString PathString = AnimMontage->GetPathName();
			FSoftObjectPath SoftPath(PathString);

			const float Dist = GetRootMotionDistanceData(SoftPath);
			MontageInfos.Emplace(AnimMontage, Dist);
		}

		Algo::Sort(MontageInfos, [DistFromTarget](TPair<UAnimMontage*, float>& A, TPair<UAnimMontage*, float>& B)
		{
			return FMath::Abs(DistFromTarget - A.Value) < FMath::Abs(DistFromTarget - B.Value);
		});

		const int32 TopK = FMath::Min(2, MontageInfos.Num());
		const int32 PickIdx = FMath::RandRange(0, TopK - 1);

		ProperMontage = MontageInfos[PickIdx].Key;
	}

	AlreadyPlayedMontage.Add(ProperMontage);

	if (IsDebugDrawing(AttackActor))
	{
		float DebugDist = CalculateRootMotionDistance(ProperMontage);

		DrawDebugSphere(AttackActor->GetWorld(), CurrentPos, 10.f, 10, FColor::Blue, false, 3.f);
		DrawDebugSphere(AttackActor->GetWorld(), CurrentPos + AttackActor->GetActorRotation().Vector() * DebugDist,
		                10.f, 10, FColor::Red, false, 3.f);
		DrawDebugLine(AttackActor->GetWorld(), CurrentPos,
		              CurrentPos + AttackActor->GetActorRotation().Vector() * DebugDist, FColor::Red, false, 3.f);
	}

	return ProperMontage;
}

AActor* FPC_GameUtil::GetBestTargetByViewAngle(APlayerController* PlayerController, TArray<AActor*> TargetActors,
                                               bool ShouldGetNotInBattleActor, float MaxAngle)
{
	AActor* FoundTarget = nullptr;
	float BestAngle = INT_MAX;

	FVector CameraForward = PlayerController->GetControlRotation().Vector();
	CameraForward.Z = 0.f;

	for (AActor* TargetActor : TargetActors)
	{
		IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(TargetActor);
		if (!CharacterInterface)
			continue;

		if (CharacterInterface->IsDead())
			continue;

		IPC_CharacterAIInterface* CharacterAIInterface = Cast<IPC_CharacterAIInterface>(CharacterInterface);
		if (!CharacterAIInterface)
			continue;

		EPC_EnemyStateType EnemyState = CharacterAIInterface->GetState();
		if (ShouldGetNotInBattleActor)
		{
			if (EnemyState != EPC_EnemyStateType::Patrol &&
				EnemyState != EPC_EnemyStateType::Investigating)
			{
				continue;
			}
		}

		FVector ToTargetDir = (TargetActor->GetActorLocation() - PlayerController->GetPawn()->GetActorLocation()).
			GetSafeNormal2D();
		float OffsetAngle = FMath::RadiansToDegrees(FMath::Acos(ToTargetDir.Dot(CameraForward)));
		if (OffsetAngle < MaxAngle)
		{
			if (OffsetAngle < BestAngle)
			{
				FoundTarget = TargetActor;
				BestAngle = OffsetAngle;
			}
		}
	}
	return FoundTarget;
}


ECollisionChannel FPC_GameUtil::GetAttackCollisionChannel(uint32 DataId)
{
	if (DataId == 0)
	{
		return ECC_GameTraceChannel3;
	}

	return ECC_GameTraceChannel4;
}

uint32 FPC_GameUtil::GetSkillId(UPC_PlayerDataAsset* PlayerDataAsset, EPC_SkillSlotType SkillSlotType,
                                EPC_CharacterStanceType StanceType, bool bInSpecialAttack)
{
	TArray<FPC_SkillEntry>& SkillIdEntries = PlayerDataAsset->SkillSlotDatas;

	for (const FPC_SkillEntry& SkillEntry : SkillIdEntries)
	{
		if (SkillEntry.Key == FPC_ComboKey(StanceType, bInSpecialAttack))
		{
			if (const uint32* FoundSkillId = SkillEntry.Data.SkillIds.Find(SkillSlotType))
			{
				return *FoundSkillId;
			}
		}
	}

	return 0;
}

void FPC_GameUtil::CameraShake(EPC_CameraShakeMagnitudeType Type)
{
	if (Type == EPC_CameraShakeMagnitudeType::None)
		return;

	UPC_GameDataAsset* GameDataAsset = GetGameData();
	if (GameDataAsset)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(
			GEngine->GetCurrentPlayWorld(), 0))
		{
			if (IsDebugDrawing(PlayerController))
				return;

			PlayerController->ClientStartCameraShake(*GameDataAsset->CameraShakeClass.Find(Type));
		}
	}
}

void FPC_GameUtil::PlayStopDilation(const UObject* WorldObject, float Duration, float Dilation)
{
	UWorld* World = WorldObject->GetWorld();
	check(World);

	if (FPC_GameUtil::IsDebugDrawing(World))
		return;

	UGameplayStatics::SetGlobalTimeDilation(World, Dilation);

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([World](float)
		{
			if (!IsValid(World))
				return false;

			UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
			return false; //다시 호출하지 않음
		}), Duration); //실제 초 기준
}

void FPC_GameUtil::PlayHitMaterial(ACharacter* DamageCharacter)
{
	IPC_CharacterInterface* CauserCharacterInterface = Cast<IPC_CharacterInterface>(DamageCharacter);
	check(CauserCharacterInterface);

	UPC_CharacterDataAsset* CharacterDataAsset = CauserCharacterInterface->GetCharacterDataAsset();
	check(CharacterDataAsset);

	if (!CharacterDataAsset->DamgeMaterial)
		return;

	USkeletalMeshComponent* SkeletalMeshComponent = DamageCharacter->GetMesh();
	check(SkeletalMeshComponent);

	UWorld* World = DamageCharacter->GetWorld();
	check(World);

	if (FPC_GameUtil::IsDebugDrawing(World))
		return;

	//기존에 머테리얼 캐싱
	UMaterialInterface* OverlayMaterial = SkeletalMeshComponent->GetOverlayMaterial();
	SkeletalMeshComponent->SetOverlayMaterial(CharacterDataAsset->DamgeMaterial);

	FTimerHandle MaterialTimer;
	World->GetTimerManager().SetTimer(
		MaterialTimer,
		[DamageCharacter,OverlayMaterial]()
		{
			if (DamageCharacter)
			{
				USkeletalMeshComponent* MeshComponent = DamageCharacter->GetMesh();
				UMaterialInterface* Material = MeshComponent->GetOverlayMaterial();

				if (MeshComponent && Material)
				{
					MeshComponent->SetOverlayMaterial(OverlayMaterial);
				}
			}
		}, 0.03f, false
	);
}

void FPC_GameUtil::PlaySFXAtLocation(UObject* WorldContextObject, USoundBase* SFX, const FVector& Location)
{
	if (!WorldContextObject || !SFX)
		return;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
		return;

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UPC_AudioSubsystem* Audio = GI->GetSubsystem<UPC_AudioSubsystem>())
		{
			Audio->PlaySFXAtLocation(SFX, Location);
		}
	}
}


// StatusType = AttackPowerUp, ValueMode = Multiplicative, ModifierValue = 1.2f (즉 +20%)
// 또는 Additive 모드라면 ModifierValue = +15.0f 같은 식
FPC_CharacterStatModifier FPC_GameUtil::MakeCharacterStatModifierFromRow(
	const FPC_StatusEffectTableRow& Row, const FPC_CharacterStatTableRow& BaseStat)
{
	FPC_CharacterStatModifier Out;

	switch (Row.StatusType)
	{
	case EPC_StatusEffectType::AttackPowerUp:
		if (Row.ValueMode == EPC_ValueMode::Multiplicative)
		{
			// BaseStat.Attack 기준으로 증가분만 계산
			const float Factor = Row.ModifierValue ? (Row.ModifierValue * 0.01f) : (Row.ModifierValue - 1.f);
			Out.AddStat.Attack = BaseStat.Attack * Factor;
		}
		else
		{
			Out.AddStat.Attack = Row.ModifierValue;
		}
		break;

	case EPC_StatusEffectType::MoveSpeed:
		if (Row.ValueMode == EPC_ValueMode::Multiplicative)
		{
			const float Factor = Row.ModifierValue ? (Row.ModifierValue * 0.01f) : (Row.ModifierValue - 1.f);
			Out.AddStat.MovementSpeed = BaseStat.MovementSpeed * Factor;
		}
		else
		{
			Out.AddStat.MovementSpeed = Row.ModifierValue;
		}
		break;

	default:
		break;
	}

	return Out;
}

void FPC_GameUtil::SpawnEffectAtLocation(UObject* WorldContextObj, UNiagaraSystem* NiagaraSystem, FVector Location,
                                         FRotator Rotation, float Scale)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObj, NiagaraSystem, Location,
	                                               Rotation, FVector(Scale));
}

void FPC_GameUtil::SpawnEffectAtLocation(UObject* WorldContextObj, UParticleSystem* ParticleSystem, FVector Location,
                                         FRotator Rotation, float Scale)
{
	UGameplayStatics::SpawnEmitterAtLocation(WorldContextObj, ParticleSystem, Location, Rotation);
}

UNiagaraComponent* FPC_GameUtil::SpawnEffectAttached(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent,
                                                     ::FName AttachPointName, FVector Location, FRotator Rotation,
                                                     EAttachLocation::Type LocationType, bool bAutoDestroy)
{
	return UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, AttachToComponent, AttachPointName, Location,
	                                                    Rotation, LocationType, bAutoDestroy);
}

UParticleSystemComponent* FPC_GameUtil::SpawnEffectAttached(UParticleSystem* ParticleSystem,
                                                            USceneComponent* AttachToComponent, FName AttachPointName,
                                                            FVector Location, FRotator Rotation,
                                                            EAttachLocation::Type LocationType, bool bAutoDestroy)
{
	return UGameplayStatics::SpawnEmitterAttached(ParticleSystem, AttachToComponent, AttachPointName, Location,
	                                              Rotation, LocationType, bAutoDestroy);
}

void FPC_GameUtil::SpawnDamageFloater(ACharacter* DamageCharacter, int32 Damge)
{
	if (!DamageCharacter) return;

	UWorld* World = DamageCharacter->GetWorld();
	check(World);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController)
		return;

	//노출될 위치
	float HalfHeight = DamageCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector WorldPosition = DamageCharacter->GetActorLocation() + FVector(0.f, 0.f, HalfHeight);

	if (GEngine)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_UISubsystem* UISubsystem = GameInstance->GetSubsystem<UPC_UISubsystem>())
			{
				UPC_DamageFloaterWidget* DamageFloaterWidget = UISubsystem->CreateDamageFloater(DamageCharacter);
				if (!DamageFloaterWidget) return;

				DamageFloaterWidget->Init(Damge, WorldPosition, PlayerController);
			}
		}
	}
}

FVector FPC_GameUtil::FindSurfacePos(ACharacter* Character, FVector& CurrentPos)
{
	UWorld* World = Character->GetWorld();
	check(World);

	//경사 높이를 계산해서 TODO (wakerable height) 로 교체
	FVector SweepStartPos = CurrentPos + FVector(0, 0, 200);
	FVector SweepEndPos = CurrentPos - FVector(0, 0, 200);

	FCollisionObjectQueryParams ObjectQueryParams;
	FCollisionQueryParams QueryParams;

	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	QueryParams.AddIgnoredActor(Character);

	float Radius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float Height = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(Radius, Height);

	FHitResult HitResult;
	World->SweepSingleByObjectType(HitResult, SweepStartPos, SweepEndPos, FQuat::Identity, ObjectQueryParams,
	                               CollisionShape, QueryParams);

	//발에 걸리면 한번 더 계산
	if (HitResult.bBlockingHit)
	{
		FVector TargetLocation = HitResult.ImpactPoint;
		FNavLocation ProjectedLocation;

		UNavigationSystemV1* NavigationSystemV1 = UNavigationSystemV1::GetCurrent(World);
		check(NavigationSystemV1);

		//그 위치가 실제로 이동 가는한 위치인지 다시 한번 확인
		bool bValid = NavigationSystemV1->ProjectPointToNavigation(
			TargetLocation, ProjectedLocation, FVector(Radius, Radius, Height) //주변 검사 범위
		);

		if (bValid)
		{
			return ProjectedLocation.Location + FVector(0, 0, Height);
		}
		else
		{
			return CurrentPos;
		}
	}

	return FVector::ZeroVector;
}

bool FPC_GameUtil::IsDebugDrawing(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();
	check(World);

	APCGameMode* GameMode = Cast<APCGameMode>(World->GetAuthGameMode());
	check(GameMode);

	return GameMode->DebugDrawing;
}

void FPC_GameUtil::AddOnScreenDebugMessage(FString msg)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, msg);
}

FString FPC_GameUtil::GetProximityString(EPC_ProximityType Proximity)
{
	switch (Proximity)
	{
	case EPC_ProximityType::Under: return TEXT("Under");
	case EPC_ProximityType::Near_l: return TEXT("Near_L");
	case EPC_ProximityType::Near_r: return TEXT("Near_R");
	case EPC_ProximityType::Front: return TEXT("Front");
	case EPC_ProximityType::Back: return TEXT("Back");
	case EPC_ProximityType::Left: return TEXT("Left");
	case EPC_ProximityType::Right: return TEXT("Right");
	case EPC_ProximityType::Far: return TEXT("Far");
	default: return TEXT("None");
	}
}

FColor FPC_GameUtil::GetProximityColor(EPC_ProximityType Proximity)
{
	switch (Proximity)
	{
	case EPC_ProximityType::Under: return FColor::Cyan;
	case EPC_ProximityType::Near_l: return FColor::Emerald;
	case EPC_ProximityType::Near_r: return FColor::Green;
	case EPC_ProximityType::Front: return FColor::Red;
	case EPC_ProximityType::Back: return FColor::Blue;
	case EPC_ProximityType::Left: return FColor::Magenta;
	case EPC_ProximityType::Right: return FColor::Yellow;
	case EPC_ProximityType::Far: return FColor::Silver;
	default: return FColor::White;
	}
}

void FPC_GameUtil::DrawProximityMapDebug(AActor* CurrentActor, float UnderRange, float NearRange,
                                         float MiddleRange, const FVector& CurrentActorOffset)
{
	if (!CurrentActor)
		return;

	UWorld* World = CurrentActor->GetWorld();
	if (!World)
		return;


	FVector Center = CurrentActor->GetActorLocation() + CurrentActorOffset;

	// 혹시 캡슐/메쉬 보정 쓰고 싶으면 여기서 추가로 조정해도 됨
	if (const ACharacter* Char = Cast<ACharacter>(CurrentActor))
	{
		Center.Z -= Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	const FVector Forward2D = CurrentActor->GetActorForwardVector().GetSafeNormal2D();
	const FVector Right2D = CurrentActor->GetActorRightVector().GetSafeNormal2D();

	// 1) 거리 링(Under / Near / Middle) 표시
	//    포폴용이니까 색 다르게 해서 한 번에 보이게
	DrawDebugSphere(World, Center, UnderRange, 32, FColor::Cyan, false, -1.f, 0, 1.f);
	DrawDebugSphere(World, Center, NearRange, 32, FColor::Yellow, false, -1.f, 0, 1.f);
	DrawDebugSphere(World, Center, MiddleRange, 32, FColor::White, false, -1.f, 0, 1.f);

	// 디버그 링 설명 텍스트
	DrawDebugString(World, Center + FVector(0, 0, 150.f),
	                TEXT("Under / Near / Middle Ranges"),
	                nullptr, FColor::White, 0.f, true);

	// 각 ProximityType 대표 위치 계산용 람다
	auto DrawSample = [&](EPC_ProximityType Type, const FVector& Dir2D, float Radius)
	{
		if (Radius <= 0.f)
			return;

		FVector Pos = Center + Dir2D.GetSafeNormal() * Radius;
		Pos.Z = Center.Z;

		const FColor Color = GetProximityColor(Type);
		const FString Text = GetProximityString(Type);

		// 작게 색 구체 + 텍스트
		DrawDebugSphere(World, Pos, 20.f, 16, Color, false, -1.f, 0, 2.f);
		DrawDebugString(World, Pos + FVector(0, 0, 40.f), Text, nullptr, Color, 0.f, true);
	};

	// 2) Under : 발 아래
	DrawSample(EPC_ProximityType::Under, FVector::ZeroVector, UnderRange * 0.3f);

	// 3) Near_l / Near_r : 대각선 전방 좌/우 근거리
	const FVector FrontLeft = (Forward2D - Right2D).GetSafeNormal();
	const FVector FrontRight = (Forward2D + Right2D).GetSafeNormal();
	DrawSample(EPC_ProximityType::Near_l, FrontLeft, NearRange * 0.7f);
	DrawSample(EPC_ProximityType::Near_r, FrontRight, NearRange * 0.7f);

	// 4) 정면 / 후면 / 좌 / 우 (MiddleRange 안쪽)
	DrawSample(EPC_ProximityType::Front, Forward2D, MiddleRange * 0.8f);
	DrawSample(EPC_ProximityType::Back, -Forward2D, MiddleRange * 0.8f);
	DrawSample(EPC_ProximityType::Left, -Right2D, MiddleRange * 0.8f);
	DrawSample(EPC_ProximityType::Right, Right2D, MiddleRange * 0.8f);

	// 5) Far : Middle 밖 한 지점 예시
	DrawSample(EPC_ProximityType::Far, Forward2D, MiddleRange * 1.3f);
}

void FPC_GameUtil::DrawProximityAngleDebug(AActor* CurrentActor, float UnderRange, float NearRange, float MiddleRange,
                                           float AngleThresholdDeg, const FVector& CurrentActorOffset)
{
	if (!CurrentActor)
		return;

	UWorld* World = CurrentActor->GetWorld();
	if (!World)
		return;

	// 중심 위치 (보스 발밑 기준 + 네가 쓰던 오프셋)
	FVector Center = CurrentActor->GetActorLocation() + CurrentActorOffset;

	if (ACharacter* Char = Cast<ACharacter>(CurrentActor))
	{
		// 발밑으로 내리기
		Center.Z -= Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	const FVector Forward2D = CurrentActor->GetActorForwardVector().GetSafeNormal2D();
	const FVector Right2D = CurrentActor->GetActorRightVector().GetSafeNormal2D();

	auto MakeDirFromAngle = [&](float AngleDeg)
	{
		const float Rad = FMath::DegreesToRadians(AngleDeg);
		// 0도 = 정면, +각도 = 오른쪽 방향으로 회전
		return (Forward2D * FMath::Cos(Rad) + Right2D * FMath::Sin(Rad)).GetSafeNormal();
	};

	auto DrawArc = [&](float Radius, float StartDeg, float EndDeg, const FColor& Color)
	{
		const int32 Segments = 32;
		FVector PrevPos;

		for (int32 i = 0; i <= Segments; ++i)
		{
			const float Alpha = static_cast<float>(i) / Segments;
			const float AngleDeg = FMath::Lerp(StartDeg, EndDeg, Alpha);

			FVector Dir = MakeDirFromAngle(AngleDeg);
			FVector Pos = Center + Dir * Radius;
			Pos.Z += 5.f; // 살짝 띄워서 지형에 묻히지 않게

			if (i > 0)
			{
				DrawDebugLine(World, PrevPos, Pos, Color, false, -1.f, 0, 2.f);
			}

			PrevPos = Pos;
		}
	};

	auto DrawSector = [&](EPC_ProximityType Type, float InnerRadius, float OuterRadius,
	                      float StartDeg, float EndDeg)
	{
		const FColor Color = GetProximityColor(Type);

		// 바깥쪽/안쪽 호 그리기
		DrawArc(OuterRadius, StartDeg, EndDeg, Color);
		DrawArc(InnerRadius, StartDeg, EndDeg, Color);

		// 양 끝쪽 반지름선
		FVector StartDir = MakeDirFromAngle(StartDeg);
		FVector EndDir = MakeDirFromAngle(EndDeg);

		FVector InnerStart = Center + StartDir * InnerRadius;
		FVector OuterStart = Center + StartDir * OuterRadius;
		FVector InnerEnd = Center + EndDir * InnerRadius;
		FVector OuterEnd = Center + EndDir * OuterRadius;

		InnerStart.Z += 10.f;
		OuterStart.Z += 10.f;
		InnerEnd.Z += 10.f;
		OuterEnd.Z += 10.f;

		DrawDebugLine(World, InnerStart, OuterStart, Color, false, -1.f, 0, 2.f);
		DrawDebugLine(World, InnerEnd, OuterEnd, Color, false, -1.f, 0, 2.f);

		// 섹터 중앙에 텍스트
		const float MidDeg = (StartDeg + EndDeg) * 0.5f;
		FVector MidDir = MakeDirFromAngle(MidDeg);
		FVector TextPos = Center + MidDir * ((InnerRadius + OuterRadius) * 0.5f);
		TextPos.Z += 40.f;

		DrawDebugString(World, TextPos, GetProximityString(Type),
		                nullptr, Color, 0.f, true, 4.f);
	};

	// 0도 = 정면, +각도 = 오른쪽
	const float FrontStart = -AngleThresholdDeg;
	const float FrontEnd = AngleThresholdDeg;

	const float RightStart = AngleThresholdDeg;
	const float RightEnd = 180.f - AngleThresholdDeg;

	const float LeftStart = -(180.f - AngleThresholdDeg);
	const float LeftEnd = -AngleThresholdDeg;

	const float BackStart = 180.f - AngleThresholdDeg;
	const float BackEnd = 180.f + AngleThresholdDeg;

	// 1) Under: 그냥 중앙 원
	if (UnderRange > 0.f)
	{
		DrawArc(UnderRange, 0.f, 360.f, GetProximityColor(EPC_ProximityType::Under));
	}

	// 2) Front / Right / Back / Left 섹터를 Near~Middle 구간에 그리기
	DrawSector(EPC_ProximityType::Front, NearRange, MiddleRange, FrontStart, FrontEnd);
	DrawSector(EPC_ProximityType::Right, NearRange, MiddleRange, RightStart, RightEnd);
	DrawSector(EPC_ProximityType::Back, NearRange, MiddleRange, BackStart, BackEnd);
	DrawSector(EPC_ProximityType::Left, NearRange, MiddleRange, LeftStart, LeftEnd);

	const float NearFrontStart = -AngleThresholdDeg;
	const float NearFrontEnd = AngleThresholdDeg;

	// 전방 왼쪽 절반
	DrawSector(EPC_ProximityType::Near_l,
	           UnderRange,
	           NearRange,
	           NearFrontStart,
	           0.f);

	// 전방 오른쪽 절반
	DrawSector(EPC_ProximityType::Near_r,
	           UnderRange,
	           NearRange,
	           0.f,
	           NearFrontEnd);
}


EPC_ProximityType FPC_GameUtil::GetTargetProximity(AActor* TargetActor, AActor* CurrentActor, float Under, float Near,
                                                   float Middle, FVector CurrentActorOffset)
{
	if (!TargetActor || !CurrentActor)
	{
		return EPC_ProximityType::None;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector CurrentLocation = CurrentActor->GetActorLocation() + CurrentActorOffset;

	const float Distance = FVector::Dist2D(TargetLocation, CurrentLocation);

	if (Distance > Middle)
	{
		return EPC_ProximityType::Far;
	}

	const FVector ForwardVector = CurrentActor->GetActorForwardVector();
	const FVector RightVector = CurrentActor->GetActorRightVector();

	const FVector ToTargetDir = (TargetLocation - CurrentLocation).GetSafeNormal();

	const float ForwardDot = FVector::DotProduct(ForwardVector, ToTargetDir);
	const float RightDot = FVector::DotProduct(RightVector, ToTargetDir);

	const float ForwardAngle = FMath::RadiansToDegrees(FMath::Acos(ForwardDot));

	if (Distance <= Under)
	{
		return EPC_ProximityType::Under;
	}

	const float AngleThreshold = 45.f;

	if (Distance <= Near && ForwardDot > 0.f)
	{
		return RightDot >= 0.f ? EPC_ProximityType::Near_r : EPC_ProximityType::Near_l;
	}

	if (Distance <= Middle)
	{
		if (ForwardAngle < AngleThreshold)
		{
			return EPC_ProximityType::Front;
		}
		else if (ForwardAngle > 180.f - AngleThreshold)
		{
			return EPC_ProximityType::Back;
		}
		else
		{
			return (RightDot >= 0.f) ? EPC_ProximityType::Right : EPC_ProximityType::Left;
		};
	}

	return EPC_ProximityType::None;
}

FColor FPC_GameUtil::GetHitPartColor(EPC_HitPartType PartType)
{
	FColor Color = FColor::White;
	switch (PartType)
	{
	case EPC_HitPartType::Body:
		Color = FColor::Green;
		break;
	case EPC_HitPartType::Arm_l:
		Color = FColor::Orange;
		break;
	case EPC_HitPartType::Arm_r:
		Color = FColor::Yellow;
		break;
	case EPC_HitPartType::Leg_l:
		Color = FColor::Cyan;
		break;
	case EPC_HitPartType::Leg_r:
		Color = FColor::Blue;
		break;
	case EPC_HitPartType::Head:
		Color = FColor::Red;
		break;
	}

	return Color;
}

FColor FPC_GameUtil::GetHitPartColor(FPC_HitPartListRow* ListRow, FName BoneName)
{
	for (FPC_HitPartData& HitPartData : ListRow->HitPartDatas)
	{
		if (HitPartData.HitPartName == BoneName)
		{
			return GetHitPartColor(HitPartData.HitPartType);
		}
	}

	return FColor::White;
}

float FPC_GameUtil::GetHitPartAddDamage(FPC_HitPartListRow* ListRow, FName BoneName)
{
	for (FPC_HitPartData& HitPartData : ListRow->HitPartDatas)
	{
		if (HitPartData.HitPartName == BoneName)
		{
			return HitPartData.AddHitDamage;
		}
	}

	return 0.0f;
}

UMaterialInterface* FPC_GameUtil::GetHitPartHitMaterial(FPC_HitPartListRow* ListRow, FName BoneName)
{
	for (FPC_HitPartData& HitPartData : ListRow->HitPartDatas)
	{
		if (HitPartData.HitPartName == BoneName)
		{
			return HitPartData.HitPartMaterial;
		}
	}

	return nullptr;
}

float FPC_GameUtil::GetCalcTotalNormalDamage(float DamageAmount, AActor* HitActor, FName BoneName)
{
	return 0;
}

FTransform FPC_GameUtil::GetSocketTransform(AActor* Actor, FName BoneName)
{
	IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(Actor);
	if (!CharacterInterface)
		return FTransform();

	ACharacter* Character = Cast<ACharacter>(Actor);
	if (!Character)
		return FTransform();

	USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh();
	if (!SkeletalMeshComponent)
		return FTransform();

	USkeletalMesh* SkeletonMesh = SkeletalMeshComponent->GetSkeletalMeshAsset();
	if (!SkeletonMesh)
		return FTransform();

	if (SkeletalMeshComponent->DoesSocketExist(BoneName))
	{
		return SkeletalMeshComponent->GetSocketTransform(BoneName);
	}
	else
	{
		UStaticMeshComponent* StaticMeshComponent_l = CharacterInterface->GetWeapon_L_StaticMeshComponent();
		UStaticMeshComponent* StaticMeshComponent_r = CharacterInterface->GetWeapon_R_StaticMeshComponent();

		if (!StaticMeshComponent_l && !StaticMeshComponent_r)
			return FTransform();

		UStaticMesh* StaticMesh = StaticMeshComponent_l->GetStaticMesh();
		if (StaticMesh)
		{
			if (StaticMesh->FindSocket(BoneName))
				return StaticMeshComponent_l->GetSocketTransform(BoneName);
		}

		StaticMesh = StaticMeshComponent_r->GetStaticMesh();
		if (StaticMesh)
		{
			if (StaticMesh->FindSocket(BoneName))
				return StaticMeshComponent_r->GetSocketTransform(BoneName);
		}

		return FTransform();
	}
}
