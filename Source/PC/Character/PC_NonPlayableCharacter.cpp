#include "PC_NonPlayableCharacter.h"

#include "Component/PC_WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PC/Utills/PC_GameUtill.h"

APC_NonPlayableCharacter::APC_NonPlayableCharacter()
{
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
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

	GetCharacterMovement()->MaxWalkSpeed = EnemyTableRow->MovementSpeed;
}

float APC_NonPlayableCharacter::GetAIPatrolRadius()
{
	return 0.f;
}

float APC_NonPlayableCharacter::GetAIDetectRange()
{
	return EnemyTableRow->DetectRange;
}

float APC_NonPlayableCharacter::GetAIAttackRange()
{
	return EnemyTableRow->AttackRange;
}

float APC_NonPlayableCharacter::GetAITurnSpeed()
{
	return 0.f;
}

void APC_NonPlayableCharacter::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == AttackAnim)
		OnAttackFinished.ExecuteIfBound();
}

void APC_NonPlayableCharacter::SetAIAttackFinishDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void APC_NonPlayableCharacter::Attack()
{
	PlayAnimMontage(AttackAnim);

	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnMontageEnd);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate);
}
