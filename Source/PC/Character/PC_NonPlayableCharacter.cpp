#include "PC_NonPlayableCharacter.h"

#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/PC_StatComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/LowLevelTestAdapter.h"
#include "PC/AI/PC_AIController.h"
#include "PC/AI/Actor/PC_PatrolRoute.h"
#include "PC/Battle/PC_NormalAttackDamageType.h"
#include "PC/Misc/GameMode/PCGameMode.h"
#include "PC/Subsystem/PC_CutsceneSubsystem.h"
#include "PC/Subsystem/PC_UISubsystem.h"
#include "PC/UI/PC_HPBarWidget.h"
#include "PC/Utills/PC_GameUtill.h"
#include "Perception/AISense_Damage.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"

class USkeletalMeshComponent;
//class ALevelSequenceActor;
APC_NonPlayableCharacter::APC_NonPlayableCharacter()
{
	IndicatorComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("IndicatorIndicatorWidgetComponent"));
	IndicatorComponent->SetupAttachment(GetMesh(), FName("Pelvis"));
	static ConstructorHelpers::FClassFinder<UUserWidget> LockOnWidgetRef(TEXT("/Game/ProjectClass/UI/WBP_Indicator.WBP_Indicator_C"));
	
	if (LockOnWidgetRef.Class)
	{
		IndicatorComponent->SetWidgetClass(LockOnWidgetRef.Class);
		IndicatorComponent->SetWidgetSpace(EWidgetSpace::Screen);
		IndicatorComponent->SetDrawSize(FVector2D(30.f, 30.f));
		IndicatorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	WidgetComponent->SetupAttachment(GetMesh());

	SkinnedDecalSampler = CreateDefaultSubobject<USkinnedDecalSampler>(TEXT("SkinnedDecalSampler"));
}

void APC_NonPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FPC_EnemyTableRow* TableRow = FPC_GameUtil::GetEnemyData(CharacterDataID))
	{
		EnemyTableRow = TableRow;
	}
	ensure(EnemyTableRow);

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		int32 RandIndex = FMath::RandRange(0, EnemyTableRow->SkeletalMeshs.Num() - 1);
		USkeletalMesh* RandomMesh = EnemyTableRow->SkeletalMeshs[RandIndex];
		MeshComponent->SetSkeletalMesh(RandomMesh);
		MeshComponent->SetAnimClass(EnemyTableRow->AnimInstance);
	}

	// 3) UI Subsystem 1회 획득
	UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);
	UPC_UISubsystem* UISubsystem = GameInstance->GetSubsystem<UPC_UISubsystem>();
	check(UISubsystem);

	if (!EnemyTableRow->IsBoss)
	{
		WidgetComponent->SetWidgetClass(UISubsystem->HPBarWidgetClass);
		WidgetComponent->InitWidget();
		WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(150.0f, 15.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	int32 NumMats = GetMesh()->GetNumMaterials();

	for (int32 i = 0; i < NumMats; i++)
	{
		UMaterialInterface* Material = GetMesh()->GetMaterial(i);
		if (UMaterialInstanceDynamic* MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(Material, this))
		{
			GetMesh()->SetMaterial(i, MaterialInstanceDynamic);
		}
	}

	SkinnedDecalSampler->UpdateAllDecals();

	if (EnemyTableRow->IsHitPartUnit)
	{
		//플레이어 Channel 무시
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
		//캡슐이 공격받는게 아니기 때문
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
	}

	GetHitPartList();
	ResetState();
}

void APC_NonPlayableCharacter::PossessedBy(AController* NewController)

{
	Super::PossessedBy(NewController);
}

float APC_NonPlayableCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                           AController* EventInstigator, AActor* DamageCauser)
{
	IPC_CharacterInterface* OwnerCharacterInterface = Cast<IPC_CharacterInterface>(this);
	check(OwnerCharacterInterface);

	IPC_CharacterInterface* CauserCharacterInterface = Cast<IPC_CharacterInterface>(DamageCauser);
	check(CauserCharacterInterface);

	UPC_CharacterDataAsset* OwnerDataAsset = OwnerCharacterInterface->GetCharacterDataAsset();
	check(OwnerDataAsset);

	UPC_CharacterDataAsset* CauseDataAsset = CauserCharacterInterface->GetCharacterDataAsset();
	check(CauseDataAsset);

	UAISense_Damage::ReportDamageEvent(
		GetWorld(),
		this,
		DamageCauser,
		DamageAmount,
		DamageCauser->GetActorLocation(),
		GetActorLocation());

	if (DamageAmount > KINDA_SMALL_NUMBER)
	{
		const FNormalAttackDamageEvent& AttackDamageEven =
			static_cast<const FNormalAttackDamageEvent&>(DamageEvent);
		
		if (UAnimInstance* AnimIns = GetMesh()->GetAnimInstance())
		{
			if (!AnimIns->IsAnyMontagePlaying())
			{
				if (EnemyState != EPC_EnemyStateType::SKillUsing && !IsDead() && !IsBossMonster)
				{
					if (OwnerDataAsset->HitReactAnim)
					{
						RequestChangeState(EPC_EnemyStateType::ReactAttackBreak);
						if (EnemyState == EPC_EnemyStateType::ReactAttackBreak)
						{
							FOnMontageEnded EndDelegate;
							EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
							{
								if(!IsValid(this))
									return;
							
								if (!IsDead())
									ChangeState(EPC_EnemyStateType::Battle);
								else
									ChangeState(EPC_EnemyStateType::Dead);
							});

							AnimIns->Montage_Play(OwnerDataAsset->HitReactAnim, 1.f, EMontagePlayReturnType::MontageLength);
							AnimIns->Montage_SetEndDelegate(EndDelegate, OwnerDataAsset->HitReactAnim);
						}
					}
				}
			}
		}

		if (DamageEvent.GetTypeID() == FPointDamageEvent::ClassID)
		{
			
			if (const FPointDamageEvent* PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent))
			{
				const FHitResult& Hit = PointEvent->HitInfo;
				
				//피격 부위 이펙트
				FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), OwnerDataAsset->HitFx, Hit.ImpactPoint,
				                                    FRotator::ZeroRotator);
				
				int32 DecalID = SkinnedDecalSampler->SpawnDecal(Hit.ImpactPoint,
				                                                Hit.ImpactNormal.Rotation().Quaternion(), Hit.BoneName,
				                                                25.0f);

				UNiagaraComponent* HitPartFXComp = nullptr;
				if (auto HitPartFX = EnemyTableRow->HitPartFX)
				{
					USkeletalMeshComponent* MeshComp = Cast<USkeletalMeshComponent>(Hit.GetComponent());
					if (MeshComp)
					{
						HitPartFXComp = FPC_GameUtil::SpawnEffectAttached(
							HitPartFX,
							MeshComp,
							Hit.BoneName,
							Hit.ImpactPoint,
							Hit.ImpactNormal.Rotation(),
							EAttachLocation::KeepWorldPosition,
							false
						);
					}
				}

				TWeakObjectPtr<UNiagaraComponent> HitPartFXWeak = HitPartFXComp;
				
				FTimerHandle TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(
					TimerHandle,
					FTimerDelegate::CreateLambda([this, DecalID, HitPartFXWeak]()
					{
						if(DecalID > 0)
						{
							if (IsValid(SkinnedDecalSampler))
							{
								SkinnedDecalSampler->RemoveDecal(DecalID);
							}
						}

						if (HitPartFXWeak.IsValid())
						{
							HitPartFXWeak->Deactivate();
							HitPartFXWeak->DestroyComponent();
						}
					}), 5.0f, false
				);
			}
			
			if (OwnerDataAsset->GroggyAnim)
			{
				if (IsHasBeenGroggy == false &&
					EnemyState != EPC_EnemyStateType::Groggy)
				{
					const float MaxHp =StatComponent->GetMaxHp();
					const float CurHp = StatComponent->GetCurrentHp();
					if (MaxHp * 0.5f >= CurHp)
					{
						RequestChangeState(EPC_EnemyStateType::Groggy);
						if (EnemyState == EPC_EnemyStateType::Groggy)
						{
							IsHasBeenGroggy = true;
							if (APC_AIController* AIController = Cast<APC_AIController>(GetController()))
							{
								EnablePhysics(true);

								AIController->StopMovement();
								AIController->StopAI();

								FOnMontageEnded EndDelegate;
								TWeakObjectPtr<APC_AIController> WeakAI = AIController;
								//기존에 머테리얼 캐싱
								USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
								check(SkeletalMeshComponent);

								UMaterialInterface* OverlayMaterial = SkeletalMeshComponent->GetOverlayMaterial();
								SkeletalMeshComponent->SetOverlayMaterial(OwnerDataAsset->GroggyMaterial);
								
								EndDelegate.BindLambda([this, WeakAI, OverlayMaterial](UAnimMontage* Montage, bool bInterrupted)
								{
									if (!IsValid(this))
										return;
								
									if (!WeakAI.IsValid())
										return;

									if (!IsDead())
									{
										USkeletalMeshComponent* MeshComponent = this->GetMesh();
										UMaterialInterface* Material = MeshComponent->GetOverlayMaterial();

										if (MeshComponent && Material)
										{
											MeshComponent->SetOverlayMaterial(OverlayMaterial);
										}
										EnablePhysics(false);
										WeakAI->RunAI();
										ChangeState(EPC_EnemyStateType::Battle);
									}
								});

								if (UAnimInstance* AnimIns = GetMesh()->GetAnimInstance())
								{
									AnimIns->StopAllMontages(0.1f);
									AnimIns->Montage_Play(OwnerDataAsset->GroggyAnim, 1.f,
														  EMontagePlayReturnType::MontageLength);
									AnimIns->Montage_SetEndDelegate(EndDelegate, OwnerDataAsset->GroggyAnim);
								}
								
							}
						}
					}
				}
				
				if (const FPointDamageEvent* PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent))
				{
					const FHitResult& Hit = PointEvent->HitInfo;
					
					if(EnemyState == EPC_EnemyStateType::Groggy)
						ApplyGroggyPhysicsReaction(Hit.BoneName, Hit.ImpactPoint);
					
					// 부위별 Hit 머테리얼 가져오기
					UMaterialInterface* HitPartOverlayMaterial =
						FPC_GameUtil::GetHitPartHitMaterial(HitPartList, Hit.BoneName);

					if (HitPartOverlayMaterial)
					{
						USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
						if (SkeletalMeshComponent)
						{
							UMaterialInterface* PrevOverlayMaterial = SkeletalMeshComponent->GetOverlayMaterial();
							SkeletalMeshComponent->SetOverlayMaterial(HitPartOverlayMaterial);
							TWeakObjectPtr<APC_BaseCharacter> WeakThis(this);

							FTimerDelegate ResetOverlayDelegate;
							ResetOverlayDelegate.BindLambda(
								[WeakThis, PrevOverlayMaterial]()
								{
									if (!WeakThis.IsValid())
										return;

									APC_BaseCharacter* Character = WeakThis.Get();
									if (!Character)
										return;

									if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
									{
										MeshComp->SetOverlayMaterial(PrevOverlayMaterial);
									}
								}
							);

							FTimerHandle ResetOverlayTimerHandle;
							GetWorldTimerManager().SetTimer(
								ResetOverlayTimerHandle,
								ResetOverlayDelegate,
								0.3f,
								false
							);
						}
					}
				}
			}
		}
		else
		{
			FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), OwnerDataAsset->HitFx, GetActorLocation(),FRotator::ZeroRotator);
		}
	}

	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return Damage;
}

void APC_NonPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsTurning && EnemyState != EPC_EnemyStateType::CrowdControlled)
	{
		if (!EnemyTableRow->IsHitPartUnit)
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				const float CurveValue = AnimInstance->GetCurveValue(TEXT("DistanceToPivot"));

				const float MaxCurveVal = -FMath::Abs(TurnDegree);

				// 3. 회전 진행률 (비율)
				float TurnAlpha = 1.f - FMath::Clamp(CurveValue / MaxCurveVal, 0.f, 1.f); // 0.0 ~ 1.0

				// 5. 회전 적용
				float FinalYaw = TurnStartYaw + (TurnDegree * TurnAlpha);
				FRotator NewRot = FRotator(0.f, FinalYaw, 0.f);
				NewRot = FMath::RInterpTo(GetActorRotation(), NewRot, DeltaTime, 10.f);
				SetActorRotation(NewRot);
			}
		}
	}

	//if (FPC_GameUtil::IsDebugDrawing(this))
	//	Tick_DrawHitPart();
}

void APC_NonPlayableCharacter::Tick_DrawHitPart()
{
	if (EnemyTableRow->IsHitPartUnit)
	{
		UWorld* World = GetWorld();
		if (!World)
			return;

		USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
		check(SkeletalMeshComponent);

		UPhysicsAsset* PhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
		if (!PhysicsAsset)
			return;

		//FSoftObjectPath AssetPath(PhysicsAsset);
		//FPC_HitPartListRow* HitPartList = FPC_GameUtil::GetHitPartData(AssetPath);
		if (!HitPartList)
			return;

		for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
		{
			if (!BodySetup)
				continue;

			const int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BodySetup->BoneName);
			if (BoneIndex == INDEX_NONE)
				continue;

			const FTransform BoneWorldTM = SkeletalMeshComponent->GetBoneTransform(BoneIndex);
			const FVector BoneScale = BoneWorldTM.GetScale3D() * 1.5f;
			const FKAggregateGeom& AggGeom = BodySetup->AggGeom;

			FColor BodyColor = FPC_GameUtil::GetHitPartColor(HitPartList, BodySetup->BoneName);

			for (const FKSphereElem& Sphere : AggGeom.SphereElems)
			{
				const FVector Center = BoneWorldTM.TransformPosition(Sphere.Center);
				const float ScaledRadius = Sphere.Radius * BoneScale.GetMax();
				DrawDebugSphere(World, Center, ScaledRadius, 16, BodyColor, false);
			}

			for (const FKBoxElem& Box : AggGeom.BoxElems)
			{
				const FTransform BoxTM = Box.GetTransform() * BoneWorldTM;
				const FVector ScaledExtent = FVector(Box.X, Box.Y, Box.Z) * 0.5f * BoneScale;
				DrawDebugBox(World, BoxTM.GetLocation(), ScaledExtent, BoxTM.GetRotation(), BodyColor, false);
			}

			for (const FKSphylElem& Sphyl : AggGeom.SphylElems)
			{
				const FTransform SphylTM = Sphyl.GetTransform() * BoneWorldTM;
				const float Scale = BoneScale.GetMax();
				const float ScaledRadius = Sphyl.Radius * Scale;
				const float ScaledLength = Sphyl.Length * Scale;

				const float HalfHeight = (ScaledLength * 0.5f) + ScaledRadius;
				DrawDebugCapsule(World, SphylTM.GetLocation(), HalfHeight, ScaledRadius, SphylTM.GetRotation(),
				                 BodyColor, false);
			}
		}
	}
}

float APC_NonPlayableCharacter::GetAIAttackRange()
{
	return EnemyTableRow->AttackRange;
}

float APC_NonPlayableCharacter::GetAITurnSpeed()
{
	return EnemyTableRow->TurnSpeed;
}

FPC_EnemyTableRow* APC_NonPlayableCharacter::GetEnemyData()
{
	if (!EnemyTableRow)
	{
		FPC_EnemyTableRow* TableRow = FPC_GameUtil::GetEnemyData(CharacterDataID);
		ensure(TableRow);

		EnemyTableRow = TableRow;
	}

	return EnemyTableRow;
}

FPC_HitPartListRow* APC_NonPlayableCharacter::GetHitPartList()
{
	if (!HitPartList)
	{
		USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
		check(SkeletalMeshComponent);

		UPhysicsAsset* PhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
		if (PhysicsAsset)
		{
			FSoftObjectPath AssetPath(PhysicsAsset);
			HitPartList = FPC_GameUtil::GetHitPartData(AssetPath);
		}
	}

	return HitPartList;
}

void APC_NonPlayableCharacter::OnAttackMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	OnAttackFinished.ExecuteIfBound();
	bLastAttacking = false;
}

void APC_NonPlayableCharacter::OnDashBackMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	OnMoveMontageFinished.ExecuteIfBound();
}

void APC_NonPlayableCharacter::OnTurnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	OnTurnFinished.ExecuteIfBound();
	IsTurning = false;
}

void APC_NonPlayableCharacter::SetAITurnFinishDelegate(const FAICharacterTurnFinished& InOnTurnFinished)
{
	OnTurnFinished = InOnTurnFinished;
}

void APC_NonPlayableCharacter::TurnInPlace(float TurnAnimDegree)
{
	check(EnemyTableRow);

	UAnimMontage* TurnAnimMontage = nullptr;

	// 45도 회전 처리
	if (TurnAnimDegree == 45.f)
	{
		if (EnemyTableRow->Left45TurnAnim)
			TurnAnimMontage = EnemyTableRow->Left45TurnAnim;
		else
			return; // 몽타주 없으면 회전 안함
	}
	else if (TurnAnimDegree == -45.f)
	{
		if (EnemyTableRow->Right45TurnAnim)
			TurnAnimMontage = EnemyTableRow->Right45TurnAnim;
		else
			return; // 몽타주 없으면 회전 안함
	}

	// 90° Left
	if (TurnAnimDegree == 90.f)
	{
		TurnAnimMontage = EnemyTableRow->Left90TurnAnim;
	}
	// 90° Right
	else if (TurnAnimDegree == -90.f)
	{
		TurnAnimMontage = EnemyTableRow->Right90TurnAnim;
	}
	// 180°
	else if (TurnAnimDegree == 180.f)
	{
		TurnAnimMontage = EnemyTableRow->Turn180Anim;
	}

	if (!TurnAnimMontage)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	check(AnimInstance);

	if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
	{
		if (CurrentMontage != TurnAnimMontage)
		{
			const float BlendOutTime = 0.2f;
			AnimInstance->Montage_Stop(BlendOutTime, CurrentMontage);
		}
	}

	IsTurning = true;
	TurnStartYaw = GetActorRotation().Yaw;
	TurnDegree = TurnAnimDegree;

	AnimInstance->Montage_Play(
		TurnAnimMontage,
		1.0f,
		EMontagePlayReturnType::MontageLength,
		0.0f,
		false
	);

	FOnMontageEnded EndDelegate =
		FOnMontageEnded::CreateUObject(this, &ThisClass::OnTurnMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate);
}

void APC_NonPlayableCharacter::DashBack()
{
	PlayAnimMontage(EnemyTableRow->DashBackAnim);

	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnDashBackMontageEnd);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate);
}

void APC_NonPlayableCharacter::SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void APC_NonPlayableCharacter::SetAIMoveMontageFinishedDelegate(
	const FAICharacterMoveMontageFinished& InOnMoveMontageFinished)
{
	OnMoveMontageFinished = InOnMoveMontageFinished;
}

void APC_NonPlayableCharacter::Attack(bool bLastAttack)
{
	check(EnemyTableRow);
	TArray<TObjectPtr<UAnimMontage>>& AttackAnims = EnemyTableRow->AttackAnims;

	AAIController* AIController = Cast<AAIController>(GetController());
	check(AIController);

	AActor* Target = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	check(Target);

	UAnimMontage* Montage = FPC_GameUtil::GetProperAttackMontage(AlreadyPlayedAttackMontages, this,
	                                                             Target->GetActorLocation());

	check(Montage);

	bLastAttacking = bLastAttack;

	if (IsDummy)
	{
		OnAttackFinished.ExecuteIfBound();
		bLastAttacking = false;
		return;
	}

	PlayAnimMontage(Montage);
	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnAttackMontageEnd);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate);
}

void APC_NonPlayableCharacter::IncrementPatrolIndex()
{
	if (APC_PatrolRoute* PatrolRouteActor = Cast<APC_PatrolRoute>(PatrolRoute))
	{
		PatrolRouteActor->IncrementIndex();
	}
}

void APC_NonPlayableCharacter::ResetState()
{
	RequestChangeState(EPC_EnemyStateType::Patrol);
}

void APC_NonPlayableCharacter::RequestChangeState(EPC_EnemyStateType StateType)
{
	if (!CanChangeState(StateType))
		return;

	ChangeState(StateType);
}

bool APC_NonPlayableCharacter::CanChangeState(EPC_EnemyStateType StateType)
{
	// 1. 이미 죽은 상태면 어떤 상태로도 전환 불가
	if (EnemyState == EPC_EnemyStateType::Dead)
		return false;

	// 2. 동일한 상태로 전환은 무의미하므로 불가
	if (EnemyState == StateType)
		return false;

	// 3. 전투 중에는 '탐색' 상태로 전환 불가
	if (EnemyState == EPC_EnemyStateType::Battle &&
		StateType == EPC_EnemyStateType::Investigating)
		return false;

	if (EnemyState == EPC_EnemyStateType::ReactAttackBreak)
		return false;

	// 4. CC(CrowdControl) 상태에서는 대부분의 상태로 전환 불가
	if (EnemyState == EPC_EnemyStateType::CrowdControlled)
	{
		switch (StateType)
		{
		case EPC_EnemyStateType::Battle:
		case EPC_EnemyStateType::SKillUsing:
		case EPC_EnemyStateType::Patrol:
		case EPC_EnemyStateType::Investigating:
			return false;
		default:
			break;
		}
	}

	// 그 외의 경우는 전환 가능
	return true;
}

void APC_NonPlayableCharacter::ChangeState(EPC_EnemyStateType StateType)
{
	if (EnemyState == EPC_EnemyStateType::Dead)
	{
		return;
	}
	
	EnemyState = StateType;
	AlreadyPlayedAttackMontages.Empty();

	if (EnemyState == EPC_EnemyStateType::Patrol || EnemyState == EPC_EnemyStateType::Investigating)
	{
		GetCharacterMovement()->MaxWalkSpeed = EnemyTableRow->MovementSpeed_Walk;
	}
	else if (EnemyState == EPC_EnemyStateType::Battle)
	{
		GetCharacterMovement()->MaxWalkSpeed = EnemyTableRow->MovementSpeed_Run;
	}
	else if (EnemyState == EPC_EnemyStateType::Groggy)
	{
		SkillComponent->ClearCurSkillList();
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsEnum(TEXT("State"), static_cast<uint8>(StateType));
		}
	}
}

void APC_NonPlayableCharacter::SetDeadType(EPC_DeadType NewDeadType)
{
	this->DeadType = NewDeadType;
}

void APC_NonPlayableCharacter::WeaponSparkEffect(bool bStart, bool bRight)
{
	Super::WeaponSparkEffect(bStart, bRight);
}

void APC_NonPlayableCharacter::OnStartCrowdControl(EPC_CrowdControlType CrowdControlType, AActor* Causer)
{
	Super::OnStartCrowdControl(CrowdControlType, Causer);
	RequestChangeState(EPC_EnemyStateType::CrowdControlled);
}

void APC_NonPlayableCharacter::OnEndCrowdControl(EPC_CrowdControlType CrowdControlType, AActor* Causer)
{
	Super::OnEndCrowdControl(CrowdControlType, Causer);

	if (!CrowdControlComponent->IsCrowdControlled())
	{
		if (AAIController* AIContoller = Cast<AAIController>(GetController()))
		{
			AIContoller->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Causer);
		}

		ChangeState(EPC_EnemyStateType::Battle);
	}
}

void APC_NonPlayableCharacter::OnDead()
{
	Super::OnDead();

	if (AAIController* AIContoller = Cast<AAIController>(GetController()))
	{
		UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(AIContoller->GetBrainComponent());
		if (BTComponent)
		{
			BTComponent->StopTree();
		}
	}

	check(CrowdControlComponent);
	CrowdControlComponent->StopCC();

	check(WidgetComponent);
	WidgetComponent->SetVisibility(false);

	check(SkillComponent)
	SkillComponent->ClearCurSkillList();

	if(OnUnlockTarget.IsBound())
		OnUnlockTarget.Broadcast(this);
	
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
		return;

	UPC_CutsceneSubsystem* CutsceneSubsystem = GI->GetSubsystem<UPC_CutsceneSubsystem>();
	if (!CutsceneSubsystem)
		return;
		
	ULevelSequence* DeathSeq = EnemyTableRow->DeathSequenceAsset.Get();
	if (!DeathSeq)
	{
		OnDeathFinished();
		return;
	}

	
	FSimpleDelegate FinishedDelegate = FSimpleDelegate::CreateUObject(
		this, &APC_NonPlayableCharacter::OnDeathFinished);

	CutsceneSubsystem->PlayCutscene(DeathSeq, FinishedDelegate);
}

void APC_NonPlayableCharacter::OnDeathFinished()
{
	RequestChangeState(EPC_EnemyStateType::Dead);

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
		Cap->SetCollisionResponseToAllChannels(ECR_Ignore);
		Cap->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); 
		Cap->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); 
		Cap->SetGenerateOverlapEvents(false); 
	}
	
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();     // Velocity 0
		MoveComp->DisableMovement();             // 더 이상 이동 안 함
		// 필요하면: MoveComp->SetMovementMode(MOVE_None);
	}

	if (UWorld* World = GetWorld())
	{
		//위젯정리
		OnLocked(false);
		OnSelectedAssassinateTarget(false);

		if(GetEnemyData()->IsBoss)
		{
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (UPC_UISubsystem* UISubsystem = GameInstance->GetSubsystem<UPC_UISubsystem>())
				{
					UISubsystem->HideBossHPWidget();
				}
			}

			if (APCGameMode* GM = World->GetAuthGameMode<APCGameMode>())
			{
				GM->PlayBGM(EPC_BGMType::Stage);
			}
		}
		
		TWeakObjectPtr<APC_NonPlayableCharacter> WeakThis(this);
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(
			DeathDestroyTimerHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->ItemDrop();
					WeakThis->Destroy();
				}
			}),
			3.0f,
			false
		);
	}
}

void APC_NonPlayableCharacter::SetupCharacterWidget(class UPC_UserWidget* InWidget)
{
	Super::SetupCharacterWidget(InWidget);

	//공통함수는 UserWidget상위로 올려도 될듯 
	if (UPC_HPBarWidget* HPBarWidget = Cast<UPC_HPBarWidget>(InWidget))
	{
		StatComponent->OnHPChangedDelegate.AddUObject(HPBarWidget, &UPC_HPBarWidget::UpdateHpBar);
		HPBarWidget->UpdateHpBar(StatComponent->GetCurrentHp(), StatComponent->GetMaxHp());
		HPBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	else if (UPC_BossHPBarWidget* BossHPBarWidget = Cast<UPC_BossHPBarWidget>(InWidget))
	{
		StatComponent->OnHPChangedDelegate.AddUObject(BossHPBarWidget, &UPC_BossHPBarWidget::UpdateHpBar);
		BossHPBarWidget->UpdateHpBar(StatComponent->GetCurrentHp(), StatComponent->GetMaxHp());
	}
}

EPC_EnemyStateType APC_NonPlayableCharacter::GetState()
{
	return EnemyState;
}

EPC_DeadType APC_NonPlayableCharacter::GetDeadType()
{
	return DeadType;
}

void APC_NonPlayableCharacter::JumpToNextAttackMontage()
{
	//마지막 공격이 아니라면
	//해당 몽타주 종료시키기
	if (!bLastAttacking)
	{
		GetMesh()->GetAnimInstance()->StopAllMontages(0.1f);
	}
}

void APC_NonPlayableCharacter::ResetUsedMontage()
{
	AlreadyPlayedAttackMontages.Empty();
}

void APC_NonPlayableCharacter::ReactAttackBreak()
{
	if (UAnimInstance* AnimIns = GetMesh()->GetAnimInstance())
	{
		if (EnemyState == EPC_EnemyStateType::Battle && !IsDead())
		{
			if (EnemyTableRow->AttackBreakAnim)
			{
				if (!CanChangeState(EPC_EnemyStateType::ReactAttackBreak))
					return;

				if (AAIController* AIContoller = Cast<AAIController>(GetController()))
				{
					AIContoller->StopMovement();
				}

				ChangeState(EPC_EnemyStateType::ReactAttackBreak);

				AnimIns->StopAllMontages(0.1f);
				AnimIns->Montage_Play(EnemyTableRow->AttackBreakAnim, 1.f, EMontagePlayReturnType::MontageLength);
				FOnMontageEnded EndDelegate;
				EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
				{
					if (!IsValid(this))
						return;
					if (!IsDead())
					{
						ChangeState(EPC_EnemyStateType::Battle);
					}
				});
				AnimIns->Montage_SetEndDelegate(EndDelegate, EnemyTableRow->AttackBreakAnim);
			}
		}
	}
}

void APC_NonPlayableCharacter::OnStartSkill(uint32 SkillId)
{
	Super::OnStartSkill(SkillId);

	RequestChangeState(EPC_EnemyStateType::SKillUsing);

	if (AAIController* AIContoller = Cast<AAIController>(GetController()))
	{
		AIContoller->StopMovement();
	}

	//GetCharacterMovement()->DisableMovement();
}

void APC_NonPlayableCharacter::OnEndSkill(uint32 SkillId)
{
	Super::OnEndSkill(SkillId);

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	check(CrowdControlComponent);

	if (!CrowdControlComponent->IsCrowdControlled())
		ChangeState(EPC_EnemyStateType::Battle);
}

bool APC_NonPlayableCharacter::IsGuarding(FVector ImpactPoint)
{
	return false;
}

bool APC_NonPlayableCharacter::IsRolling()
{
	return false;
}

//void APC_NonPlayableCharacter::EnablePhysics(bool Enable)
//{
//	USkeletalMeshComponent* SkeletalMesh = GetMesh();
//	if (!SkeletalMesh)
//		return;
//
//	if(Enable)
//	{
//		SkeletalMesh->SetAllBodiesBelowSimulatePhysics(GroggyPhysicsRootBone, true, false);
//		SkeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(GroggyPhysicsRootBone, 0.3f, false, true);
//	}
//	else
//	{
//		SkeletalMesh->SetAllBodiesBelowSimulatePhysics(GroggyPhysicsRootBone, false, false);
//		SkeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(GroggyPhysicsRootBone, 0.0f, false, true);
//	}
//
//}

void APC_NonPlayableCharacter::EnablePhysics(bool bEnable)
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (!SkeletalMesh)
		return;
	
	if (bEnable)
	{
		// 다리/골반은 건드리지 말고, 상체 루트만 부분 피직스
		SkeletalMesh->SetAllBodiesBelowSimulatePhysics(GroggyPhysicsRootBone, true, false);
		SkeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(GroggyPhysicsRootBone, 0.6f, false, true);
		// 0.3f = 애님 70% / 피직스 30% → 기본 포즈(쭈그리기) 유지 + 살짝 흔들
	}
	else
	{
		SkeletalMesh->SetAllBodiesBelowSimulatePhysics(GroggyPhysicsRootBone, false, false);
		SkeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(GroggyPhysicsRootBone, 0.0f, false, true);
	}
}


void APC_NonPlayableCharacter::ApplyGroggyPhysicsReaction(const FName& BoneName, FVector HitLocation) const
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (!SkeletalMesh)
		return;

	// 항상 몸통 루트 기준으로 들썩이게
	const FName RootBone = GroggyPhysicsRootBone;//"spine_01";//GroggyPhysicsRootBone; // spine_02 같은 것

	// 맞은 위치 → 방향 계산용
	FVector ImpulseDir = GetActorLocation() - HitLocation;
	//ImpulseDir.Z = 0.f; // 수평 방향만 사용 (위/아래로 안 날아가게)
	//ImpulseDir.Z *= 0.2f; // 너무 튀지 않게 20%만 허용
	if (!ImpulseDir.Normalize())
		return;

	// 너무 심하게 안 흔들리게 적당히 작은 값부터 시작
	const float ImpulsePower = 113000.0f; // 150~400 사이 튜닝
	const FVector Impulse = ImpulseDir * ImpulsePower;

	SkeletalMesh->AddImpulseToAllBodiesBelow(
		Impulse,
		RootBone, 
		false     
	);
}

void APC_NonPlayableCharacter::ItemDrop()
{
	FPC_EnemyTableRow* EnemyData = GetEnemyData();
	if (EnemyData->SpawnItem.IsValid())
	{
		UClass* SpawnClass = EnemyData->SpawnItem.Get();
		if (!SpawnClass)
			return;

		UWorld* World = GetWorld();
		if (!World)
			return;
						
		FVector SpawnLoc = GetActorLocation();
		const FRotator SpawnRotation = GetActorRotation();
		FVector TraceStartPos = SpawnLoc + FVector(0, 0, 300.f);
		FVector TraceEndPos = SpawnLoc - FVector(0, 0, 1000.f);

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

		FHitResult HitResult;
		World->LineTraceSingleByObjectType(HitResult, TraceStartPos, TraceEndPos, ObjectQueryParams);
		SpawnLoc = HitResult.ImpactPoint;
		AActor* Spawned = World->SpawnActor<AActor>(SpawnClass, SpawnLoc, SpawnRotation);
	}
}

void APC_NonPlayableCharacter::EnableHPBar(bool bEnable)
{
	if (!IsValid(WidgetComponent))
		return;
	
	if (UUserWidget* Widget = WidgetComponent->GetWidget())
		Widget->SetVisibility(bEnable? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

AActor* APC_NonPlayableCharacter::GetPatrolRoute()
{
	return PatrolRoute;
}
