// Copyright Epic Games, Inc. All Rights Reserved.

#include "PC_BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Component/PC_StatComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "Component/PC_BattleComponent.h"
#include "Component/PC_SkillComponent.h"
#include "PC/PC.h"
#include "PC/UI/PC_AttackIndicatorWidget.h"
#include "PC/UI/PC_LockOnWidget.h"
#include "PC/UI/PC_HPBarWidget.h"
#include "PC/Utills/PC_GameUtill.h"

// APCCharacter


APC_BaseCharacter::APC_BaseCharacter()
{
// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	
	BattleComponent = CreateDefaultSubobject<UPC_BattleComponent>(TEXT("BattleComponent"));
	CrowdControlComponent = CreateDefaultSubobject<UPC_CrowdControlComponent>(TEXT("CrowdControlComponent"));
	StatComponent = CreateDefaultSubobject<UPC_StatComponent>(TEXT("StatComponent"));
	
	Weapon_L_StaticComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_L_StaticComponent"));
	Weapon_L_StaticComponent->SetupAttachment(GetMesh());
	Weapon_R_StaticComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_R_StaticComponent"));
	Weapon_R_StaticComponent->SetupAttachment(GetMesh());

	SkillComponent = CreateDefaultSubobject<UPC_SkillComponent>(TEXT("SkillComponent"));
	WidgetComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetMesh());
}

void APC_BaseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

float APC_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	StatComponent->ApplyDamage(DamageAmount);

	FPC_GameUtil::PlayHitMaterial(this);
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void APC_BaseCharacter::ApplyStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat)
{
	float MovementSpeed = (BaseStat + ModifierStat).MovementSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void APC_BaseCharacter::SetupCharacterWidget(UPC_UserWidget* InWidget)
{

}

void APC_BaseCharacter::SetupLockOnWidget(UPC_UserWidget* InUserWidget)
{
	if (UPC_LockOnWidget* LockOnWidget = Cast<UPC_LockOnWidget>(InUserWidget))
	{
		OnCharacterLocked.AddDynamic(LockOnWidget, &UPC_LockOnWidget::ToggleActivation);
	}
}

void APC_BaseCharacter::SetupAttackIndicatorOnWidget(class UPC_UserWidget* InUserWidget)
{
	if (UPC_AttackIndicatorWidget* OnWidget = Cast<UPC_AttackIndicatorWidget>(InUserWidget))
	{
		OnAttackIndicatorChanged.AddDynamic(OnWidget, &UPC_AttackIndicatorWidget::ToggleActivation);
	}
}

void APC_BaseCharacter::OnLocked(bool bLocked)
{
	OnCharacterLocked.Broadcast(bLocked);
}

void APC_BaseCharacter::LaunchCharacter(FVector StartPos, FVector CauserPos, float Power)
{
	FVector Dir2D = (StartPos - CauserPos).GetSafeNormal2D();
	FVector Target = GetActorLocation() + Dir2D * Power; // Distance=수 cm~수십 cm
	FVector NewPos = FPC_GameUtil::FindSurfacePos(this, Target);
	SetActorLocation(NewPos, true);
	
	//const FVector RawDir = (StartPos - CauserPos).GetSafeNormal2D();
	//const FVector FloorNormal = GetCharacterMovement()->CurrentFloor.HitResult.ImpactNormal;
	//const FVector GroundDir = FVector::VectorPlaneProject(RawDir, FloorNormal).GetSafeNormal2D();
	//
	//Super::LaunchCharacter(GroundDir* Power, true, false);
}

void APC_BaseCharacter::OnAttackIndicator(bool bAttackIndicator)
{
	OnAttackIndicatorChanged.Broadcast(bAttackIndicator);
}

void APC_BaseCharacter::OnDead()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	check(AnimInstance);

	AnimInstance->StopAllMontages(0.f);
	
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionProfileName(EName::Pawn);
}

bool APC_BaseCharacter::IsDead()
{
	check(StatComponent);
	return StatComponent->CurrentHp < KINDA_SMALL_NUMBER;
}

TPair<FName, FName> APC_BaseCharacter::GetWeaponTraceNames(bool bRight)
{
	return {BattleComponent->TraceEndBoneName, BattleComponent->TraceEndBoneName};
}

FPC_OnStartSkillDelegate& APC_BaseCharacter::GetOnStartSkillDelegate()
{
	check(SkillComponent);
	return SkillComponent->OnStartSkillDelegate;
}

FPC_OnEndSkillDelegate& APC_BaseCharacter::GetOnEndSkillDelegate()
{
	check(SkillComponent);
	return SkillComponent->OnEndSkillDelegate;
}

void APC_BaseCharacter::OnStartCrowdControl(EPC_CrowdControlType CrowdType, AActor* actor)
{
}

void APC_BaseCharacter::OnEndCrowdControl(EPC_CrowdControlType CrowdType, AActor* actor)
{
}

void APC_BaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StatComponent->OnCharacterDieDelegate.AddUObject(this, &ThisClass::OnDead);
	StatComponent->OnStatChangedDelegate.AddUObject(this, &ThisClass::ApplyStat);
}

void APC_BaseCharacter::AttackTrace(bool bStart, FName TraceStartBoneName, FName TraceEndBoneName)
{
	check(BattleComponent);
	
	if (bStart)
		BattleComponent->StartTrace(TraceStartBoneName, TraceEndBoneName);
	else
		BattleComponent->EndTrace();
}

void APC_BaseCharacter::AttackTraceWithWeapon(bool bStart, bool bRight)
{
	if (bStart)
		BattleComponent->StartTraceWithWeapon(bRight);
	else
		BattleComponent->EndTrace();
}

bool APC_BaseCharacter::HasWeapon()
{
	check(BattleComponent);
	return BattleComponent->HasWeapon();
}
//



