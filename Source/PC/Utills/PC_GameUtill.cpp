#include "PC_GameUtill.h"

#include "NiagaraFunctionLibrary.h"
#include "Logging/LogMacros.h"
#include  "CoreMinimal.h"
#include "PC/PC.h"
#include "PC/Cometic/PC_LegacyCameraShake.h"
#include "CoreTypes.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "DSP/MidiNoteQuantizer.h"
#include "DynamicMesh/MeshTransforms.h"
#include "GameFramework/Character.h"
#include "PC/Data/PC_CharacterDataAsset.h"
#include "PC/Data/PC_HitPartDataAsset.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/Misc/GameMode/PCGameMode.h"
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

UAnimMontage* FPC_GameUtil::GetProperAttackMontage(TArray<TObjectPtr<UAnimMontage>>& AnimMontages,
                                                   TArray<TObjectPtr<UAnimMontage>>& AlreadyPlayedMontage,
                                                   AActor* AttackActor, FVector TargetPos)
{
	auto BuildCandidates = [&]()-> TArray<UAnimMontage*>
	{
		TArray<UAnimMontage*> Out;
		Out.Reserve(AnimMontages.Num());

		for (UAnimMontage* Montage : AnimMontages)
		{
			if (Montage && !AlreadyPlayedMontage.Contains(Montage))
			{
				Out.Add(Montage);
			}
		}
		return Out;
	};

	TArray<UAnimMontage*> Candidates = BuildCandidates();

	//후보군이 없으면
	if (Candidates.Num() == 0)
	{
		//마지막 했던거 제외 시키도록
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
			{
				return AnimMontages[0];
			}

			return nullptr;
		}
	}

	FVector CurrentPos = AttackActor->GetActorLocation();
	float DisFromTarget = FVector::Dist(CurrentPos, TargetPos);

	TArray<TPair<UAnimMontage*, float>> Montages;
	Montages.Reserve(Candidates.Num());

	TArray<FPC_AnimMontageRootMotionDistanceRow*> RootMotionDistanceTableRows = GetAllRows<
		FPC_AnimMontageRootMotionDistanceRow>(EPC_DataTableType::RootMotionDistance);
	for (UAnimMontage* Montage : Candidates)
	{
		FString PathStr = Montage->GetPathName();
		FSoftObjectPath SoftObjectPath(PathStr);
		const float Dist = GetRootMotionDistanceData(SoftObjectPath);
		Montages.Emplace(Montage, Dist);
	}

	//Algo::Sort(Montages, [DisFromTarget](TPair<UAnimMontage*, float>& A, TPair<UAnimMontage*, float>& B)
	//{
	//	return FMath::Abs(DisFromTarget - A.Value < FMath::Abs(DisFromTarget - B.Value));
	//});

	Algo::Sort(Montages, [DisFromTarget](TPair<UAnimMontage*, float>& A, TPair<UAnimMontage*, float>& B)
	{
		return FMath::Abs(DisFromTarget - A.Value) < FMath::Abs(DisFromTarget - B.Value);
	});


	const int32 TopK = FMath::Min(1, Montages.Num()); //몽타주 풀이 작을수도.
	const int32 PickIdx = FMath::RandRange(0, TopK - 1);

	UAnimMontage* ProperMontage = Montages[PickIdx].Key;
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
			PlayerController->ClientStartCameraShake(*GameDataAsset->CameraShakeClass.Find(Type));
		}
	}
}

void FPC_GameUtil::PlayStopDilation(const UObject* WorldObject, float Duration, float Dilation)
{
	UWorld* World = WorldObject->GetWorld();
	check(World);

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

	USkeletalMeshComponent* SkeletalMeshComponent = DamageCharacter->GetMesh();
	check(SkeletalMeshComponent);

	UWorld* World = DamageCharacter->GetWorld();
	check(World);

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

//TODO 풀링 처리
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

EPC_ProximityType FPC_GameUtil::GetTargetProximity(AActor* TargetActor, AActor* CurrentActor, float Near, float Middle)
{
	if (!TargetActor || !CurrentActor)
	{
		return EPC_ProximityType::None;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector CurrentLocation = CurrentActor->GetActorLocation();

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
	UE_LOG(LogTemp, Log, TEXT("%f"), Distance);
	if (Distance <= Near)
	{
		return RightDot >= 0.f ? EPC_ProximityType::Near_r : EPC_ProximityType::Near_l;
	}

	if (Distance <= Middle)
	{
		const float ForwardThres = 0.7f; // cos(45°) ≈ 0.707

		if (ForwardDot >= ForwardThres)
			return EPC_ProximityType::Front;
		else if (ForwardDot <= -ForwardThres)
			return EPC_ProximityType::Back;
		else
			return RightDot >= 0.f ? EPC_ProximityType::Right : EPC_ProximityType::Left;
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

		if(!StaticMeshComponent_l && !StaticMeshComponent_r)
			return FTransform();

		UStaticMesh* StaticMesh = StaticMeshComponent_l->GetStaticMesh();
		if(StaticMesh)
		{
			if(StaticMesh->FindSocket(BoneName))
				return StaticMeshComponent_l->GetSocketTransform(BoneName);
		}
		
		StaticMesh = StaticMeshComponent_r->GetStaticMesh();
		if(StaticMesh)
		{
			if(StaticMesh->FindSocket(BoneName))
				return StaticMeshComponent_r->GetSocketTransform(BoneName);
		}

		return FTransform();

	}
}
