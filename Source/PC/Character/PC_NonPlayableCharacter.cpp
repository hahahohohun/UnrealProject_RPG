#include "PC_NonPlayableCharacter.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PC/AI/PC_AIController.h"
#include "PC/AI/Actor/PC_PatrolRoute.h"
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
	
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
}

void APC_NonPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FPC_EnemyTableRow* TableRow = FPC_GameUtil::GetEnemyData(CharacterType))
	{
		EnemyTableRow = TableRow;
	}
	ensure(EnemyTableRow);

	if (USkeletalMeshComponent* MeshComponent = GetMesh()) 
	{
		MeshComponent->SetSkeletalMesh(EnemyTableRow->SkeletalMesh);
		MeshComponent->SetAnimClass(EnemyTableRow->AnimInstance);
	}

	GetCharacterMovement()->MaxWalkSpeed = EnemyTableRow->MovementSpeed_Walk;
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

	if (IsTurning)
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
		FPC_EnemyTableRow* TableRow = FPC_GameUtil::GetEnemyData(CharacterType);
		ensure(TableRow);

		EnemyTableRow = TableRow;
	}
	
	return EnemyTableRow;
}

void APC_NonPlayableCharacter::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == AttackAnim)
	{
		OnAttackFinished.ExecuteIfBound();
	}
	else if (Montage == TurnAnimMontage)
	{
		OnTurnFinished.ExecuteIfBound();
		TurnAnimMontage = nullptr;
		IsTurning = false;
	}
}

void APC_NonPlayableCharacter::SetAITurnFinishDelegate(const FAICharacterTurnFinished& InOnTurnFinished)
{
	OnTurnFinished = InOnTurnFinished;
}

void APC_NonPlayableCharacter::TurnInPlace(float TurnAnimDegree)
{
	check(EnemyTableRow);
	if (TurnAnimDegree == 90.f)
		TurnAnimMontage = EnemyTableRow->Left90TurnAnim;
	else if (TurnAnimDegree == 180.f)
		TurnAnimMontage = EnemyTableRow->Left180TurnAnim;
	else if (TurnAnimDegree == -90.f)
		TurnAnimMontage = EnemyTableRow->Right90TurnAnim;
	else if (TurnAnimDegree == -180.f)
		TurnAnimMontage = EnemyTableRow->Right180TurnAnim;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	check(AnimInstance);

	IsTurning = true;
	TurnStartYaw = GetActorRotation().Yaw;
	TurnDegree = TurnAnimDegree;

	PlayAnimMontage(TurnAnimMontage);

	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnMontageEnd);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate);
}

void APC_NonPlayableCharacter::SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void APC_NonPlayableCharacter::Attack()
{
	check(EnemyTableRow->AttackAnim);
	PlayAnimMontage(EnemyTableRow->AttackAnim);
	AttackAnim = EnemyTableRow->AttackAnim;

	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnMontageEnd);
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
	EnemyState = StateType;

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

EPC_EnemyStateType APC_NonPlayableCharacter::GetState()
{
	return EnemyState;	
}

AActor* APC_NonPlayableCharacter::GetPatrolRoute()
{
	return PatrolRoute;
}


