#include "PC_NonPlayableCharacter.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/PC_StatComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/LowLevelTestAdapter.h"
#include "PC/AI/PC_AIController.h"
#include "PC/AI/Actor/PC_PatrolRoute.h"
#include "PC/Subsystem/PC_UISubsystem.h"
#include "PC/UI/PC_HPBarWidget.h"
#include "PC/Utills/PC_GameUtill.h"
#include "Perception/AISense_Damage.h"

APC_NonPlayableCharacter::APC_NonPlayableCharacter()
{
	IndicatorComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("IndicatorIndicatorWidgetComponent"));
	IndicatorComponent->SetupAttachment(GetMesh(), FName("Pelvis"));
	static ConstructorHelpers::FClassFinder<UUserWidget> LockOnWidgetRef(TEXT("/Game/ProjectClass/UI/WBP_Indicator.WBP_Indicator_C"));
	if(LockOnWidgetRef.Class)
	{
		IndicatorComponent->SetWidgetClass(LockOnWidgetRef.Class);
		IndicatorComponent->SetWidgetSpace(EWidgetSpace::Screen);
		IndicatorComponent->SetDrawSize(FVector2D(30.f, 30.f));
		IndicatorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	WidgetComponent->SetRelativeLocation(FVector(0.0f,0.0f,0.0f));
	WidgetComponent->SetupAttachment(GetMesh());
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
		MeshComponent->SetSkeletalMesh(EnemyTableRow->SkeletalMesh);
		MeshComponent->SetAnimClass(EnemyTableRow->AnimInstance);
	}

	// 3) UI Subsystem 1회 획득
	UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);
	UPC_UISubsystem* UISubsystem = GameInstance->GetSubsystem<UPC_UISubsystem>();
	check(UISubsystem);

	if(!EnemyTableRow->IsBoss)
	{
		WidgetComponent->SetWidgetClass(UISubsystem->HPBarWidgetClass);
		WidgetComponent->InitWidget();
		WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(150.0f, 15.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if(EnemyTableRow->IsHitPartUnit)
	{
		//플레이어 Channel 무시
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
		//캡슐이 공격받는게 아니기 때문
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
	}
	
	ResetState();
}

void APC_NonPlayableCharacter::PossessedBy(AController* NewController)

{
	Super::PossessedBy(NewController);
}

float APC_NonPlayableCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	UAISense_Damage::ReportDamageEvent(
	GetWorld(), 
	this,
	DamageCauser,
	DamageAmount,
	DamageCauser->GetActorLocation(),
	GetActorLocation());

	if(DamageAmount > KINDA_SMALL_NUMBER)
	{
		FPC_GameUtil::SpawnDamageFloater(this, DamageAmount);

		if(UAnimInstance* AnimIns = GetMesh()->GetAnimInstance())
		{
			if(EnemyState != EPC_EnemyStateType::SKillUsing && !IsDead()
				&& !EnemyTableRow->HasSuperAmor)
			{
				if(EnemyTableRow->HitReactAnim)
				{
					AnimIns->Montage_Play(EnemyTableRow->HitReactAnim, 1.f, EMontagePlayReturnType::MontageLength);
				}
			}
		}
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void APC_NonPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsTurning && EnemyState != EPC_EnemyStateType::CrowdControlled)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			const float CurveValue = AnimInstance->GetCurveValue(TEXT("DistanceToPivot"));
			UE_LOG(LogTemp, Log, TEXT("%.2f"), CurveValue);
			//에셋 자체가 -1이기 때문에
			const float MaxCurveVal = -FMath::Abs(TurnDegree);
			
			// 3. 회전 진행률 (비율)
			float TurnAlpha = 1.f - FMath::Clamp(CurveValue / MaxCurveVal, 0.f, 1.f);  // 0.0 ~ 1.0
			
			// 5. 회전 적용
			float FinalYaw = TurnStartYaw + (TurnDegree * TurnAlpha);
			FRotator NewRot = FRotator(0.f, FinalYaw, 0.f);
			NewRot = FMath::RInterpTo(GetActorRotation(), NewRot, DeltaTime, 10.f);
			SetActorRotation(NewRot);
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

void APC_NonPlayableCharacter::OnAttackMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	OnAttackFinished.ExecuteIfBound();
	bLastAttacking = false;
}

void APC_NonPlayableCharacter::OnDashBackMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	OnMoveMontageFinished.ExecuteIfBound();
}

void APC_NonPlayableCharacter::SetAITurnFinishDelegate(const FAICharacterTurnFinished& InOnTurnFinished)
{
	OnTurnFinished = InOnTurnFinished;
}

void APC_NonPlayableCharacter::TurnInPlace(float TurnAnimDegree)
{
	//check(EnemyTableRow);
	//if (TurnAnimDegree == 90.f)
	//	TurnAnimMontage = EnemyTableRow->Left90TurnAnim;
	//else if (TurnAnimDegree == 180.f)
	//	TurnAnimMontage = EnemyTableRow->Left180TurnAnim;
	//else if (TurnAnimDegree == -90.f)
	//	TurnAnimMontage = EnemyTableRow->Right90TurnAnim;
	//else if (TurnAnimDegree == -180.f)
	//	TurnAnimMontage = EnemyTableRow->Right180TurnAnim;
	//
	//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//check(AnimInstance);
	//
	//IsTurning = true;
	//TurnStartYaw = GetActorRotation().Yaw;
	//TurnDegree = TurnAnimDegree;
	//
	//PlayAnimMontage(TurnAnimMontage);
	//
	//FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnAttackMontageEnd);
	//GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate);
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

void APC_NonPlayableCharacter::SetAIMoveMontageFinishedDelegate(const FAICharacterMoveMontageFinished& InOnMoveMontageFinished)
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
	
	UAnimMontage* Montage = FPC_GameUtil::GetProperAttackMontage(AttackAnims, AlreadyPlayedAttackMontages,
		this, Target->GetActorLocation() );
	
	check(Montage);
	
	bLastAttacking = bLastAttack;
	
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
	if(!CanChangeState(StateType))
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
	if(EnemyState == EPC_EnemyStateType::Dead)
	{
		return;	
	}

	EnemyState = StateType;
	AlreadyPlayedAttackMontages.Empty();
	
	if (EnemyState == EPC_EnemyStateType::Patrol || EnemyState == EPC_EnemyStateType::Investigating )
	{
		GetCharacterMovement()->MaxWalkSpeed = EnemyTableRow->MovementSpeed_Walk;
	}
	else if (EnemyState == EPC_EnemyStateType::Battle)
	{
		GetCharacterMovement()->MaxWalkSpeed = EnemyTableRow->MovementSpeed_Run;
	}

	if(AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if(UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsEnum(TEXT("State"), static_cast<uint8>(StateType));
		}
	}
}

void APC_NonPlayableCharacter::SetDeadType(EPC_DeadType NewDeadType)
{
	this->DeadType = NewDeadType;
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
		
		RequestChangeState(EPC_EnemyStateType::Battle);
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
	
	RequestChangeState(EPC_EnemyStateType::Dead);

	if(UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);    // 물리 충돌 X, 트레이스만
		Cap->SetCollisionResponseToAllChannels(ECR_Ignore);
		Cap->SetCollisionResponseToChannel(ECC_WorldStatic,  ECR_Block);   // 바닥/벽
		Cap->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 필요 시 시야/트레이스 유지
		Cap->SetGenerateOverlapEvents(false); // 겹침 이벤트 필요 없으면
	}
}

void APC_NonPlayableCharacter::SetupCharacterWidget(class UPC_UserWidget* InWidget)
{
	Super::SetupCharacterWidget(InWidget);

	//공통함수는 UserWidget상위로 올려도 될듯 
	if(UPC_HPBarWidget* HPBarWidget = Cast<UPC_HPBarWidget>(InWidget))
	{
		StatComponent->OnHPChangedDelegate.AddUObject(HPBarWidget, &UPC_HPBarWidget::UpdateHpBar);
		HPBarWidget->UpdateHpBar(StatComponent->GetCurrentHp(), StatComponent->GetMaxHp());
	}
	else if(UPC_BossHPBarWidget* BossHPBarWidget = Cast<UPC_BossHPBarWidget>(InWidget))
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
	if(!bLastAttacking)
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
	if(UAnimInstance* AnimIns = GetMesh()->GetAnimInstance())
	{
		if(EnemyState == EPC_EnemyStateType::Battle && !IsDead())
		{
			if(EnemyTableRow->AttackBreakAnim)
			{
				if(!CanChangeState(EPC_EnemyStateType::ReactAttackBreak))
					return;
				
				if(AAIController* AIContoller = Cast<AAIController>(GetController()))
				{
					AIContoller->StopMovement();
				}
				
				ChangeState(EPC_EnemyStateType::ReactAttackBreak);
				
				AnimIns->StopAllMontages(0.1f);
				AnimIns->Montage_Play(EnemyTableRow->AttackBreakAnim, 1.f, EMontagePlayReturnType::MontageLength);
				FOnMontageEnded EndDelegate;
				EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
				{
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
	
	if(AAIController* AIContoller = Cast<AAIController>(GetController()))
	{
		AIContoller->StopMovement();
	}

	GetCharacterMovement()->DisableMovement();
}

void APC_NonPlayableCharacter::OnEndSkill(uint32 SkillId)
{
	Super::OnEndSkill(SkillId);

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	check(CrowdControlComponent);
	
	if(!CrowdControlComponent->IsCrowdControlled())
		ChangeState(EPC_EnemyStateType::Battle);
}

AActor* APC_NonPlayableCharacter::GetPatrolRoute()
{
	return PatrolRoute;
}


