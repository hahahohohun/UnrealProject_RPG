#include "PC_NonPlayableCharacter.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/PC_BackstabSystemComponent.h"
#include "Component/PC_StatComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PC/AI/PC_AIController.h"
#include "PC/AI/Actor/PC_PatrolRoute.h"
#include "PC/Subsystem/PC_UISubsystem.h"
#include "PC/UI/PC_HPBarWidget.h"
#include "PC/Utills/PC_GameUtill.h"
#include "Perception/AISense_Damage.h"

APC_NonPlayableCharacter::APC_NonPlayableCharacter()
{
	LockOnWidgetComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("LockOnWidgetComponent"));
	LockOnWidgetComponent->SetupAttachment(GetMesh(), FName("Pelvis"));
	static ConstructorHelpers::FClassFinder<UUserWidget> LockOnWidgetRef(TEXT("/Game/ProjectClass/UI/WBP_LockOn.WBP_LockOn_C"));
	if(LockOnWidgetRef.Class)
	{
		LockOnWidgetComponent->SetWidgetClass(LockOnWidgetRef.Class);
		LockOnWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		LockOnWidgetComponent->SetDrawSize(FVector2D(30.f, 30.f));
		LockOnWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AttackIndicatorWidgetComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("AttackIndicatorWidgetComponent"));
	AttackIndicatorWidgetComponent->SetupAttachment(GetMesh(), FName("neck_01"));
	static ConstructorHelpers::FClassFinder<UUserWidget> AttackIndicatorWidgetRef(TEXT("/Game/ProjectClass/UI/WBP_AttackIndicator.WBP_AttackIndicator_C"));
	if(AttackIndicatorWidgetRef.Class)
	{
		AttackIndicatorWidgetComponent->SetWidgetClass(AttackIndicatorWidgetRef.Class);
		AttackIndicatorWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		AttackIndicatorWidgetComponent->SetDrawSize(FVector2D(30.f, 30.f));
		AttackIndicatorWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	if(EnemyTableRow->IsBoos)
	{
		
	}
	else
	{
		UGameInstance* GameInstance = GetGameInstance();
		check(GameInstance);

		UPC_UISubsystem* UISubsystem = GameInstance->GetSubsystem<UPC_UISubsystem>();
		check(UISubsystem);
		
		WidgetComponent->SetWidgetClass(UISubsystem->HPBarWidgetClass);
		WidgetComponent->InitWidget();

		WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(150.0f, 15.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	AActor* TargetActor = Cast<AAIController>(GetController());
	check(TargetActor);

	UAnimMontage* Montage = FPC_GameUtil::GetProperAttackMontage(AttackAnims, AlreadyPlayedAttackMontages,
		this, TargetActor->GetActorLocation() );
	
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
	ChangeState(EPC_EnemyStateType::Patrol);
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
		AIController->GetBlackboardComponent()->SetValueAsEnum(TEXT("State"), static_cast<uint8>(StateType));
	}
}

void APC_NonPlayableCharacter::SetDeadType(EPC_DeadType NewDeadType)
{
	this->DeadType = NewDeadType;
}

void APC_NonPlayableCharacter::OnStartCrowdControl(EPC_CrowdControlType CrowdControlType, AActor* Causer)
{
	Super::OnStartCrowdControl(CrowdControlType, Causer);
	ChangeState(EPC_EnemyStateType::CrowdControlled);
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
	
	ChangeState(EPC_EnemyStateType::Dead);

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

AActor* APC_NonPlayableCharacter::GetPatrolRoute()
{
	return PatrolRoute;
}


