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
#include "Engine/DamageEvents.h"
#include "PC/PC.h"
#include "PC/Battle/PC_NormalAttackDamageType.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/UI/PC_IndicatorWidget.h"
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
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;

	BattleComponent = CreateDefaultSubobject<UPC_BattleComponent>(TEXT("BattleComponent"));
	CrowdControlComponent = CreateDefaultSubobject<UPC_CrowdControlComponent>(TEXT("CrowdControlComponent"));
	StatComponent = CreateDefaultSubobject<UPC_StatComponent>(TEXT("StatComponent"));
	StatusEffectComponent = CreateDefaultSubobject<UPC_StatusEffectComponent>(TEXT("StatusEffectComponent"));

	Weapon_L_StaticComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_L_StaticComponent"));
	Weapon_L_StaticComponent->SetupAttachment(GetMesh());
	Weapon_R_StaticComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_R_StaticComponent"));
	Weapon_R_StaticComponent->SetupAttachment(GetMesh());

	SkillComponent = CreateDefaultSubobject<UPC_SkillComponent>(TEXT("SkillComponent"));
	WidgetComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetMesh());

	StatusEffectWidgetComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("StatusEffectWidgetComponent"));
	StatusEffectWidgetComponent->SetupAttachment(GetMesh(), FName("Pelvis"));
	static ConstructorHelpers::FClassFinder<UUserWidget> StatusEffectWidgetComponentRef(
		TEXT("/Game/ProjectClass/UI/WBP_StatusEffect.WBP_StatusEffect_C"));
	if (StatusEffectWidgetComponentRef.Class)
	{
		StatusEffectWidgetComponent->SetWidgetClass(StatusEffectWidgetComponentRef.Class);
		StatusEffectWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		StatusEffectWidgetComponent->SetDrawSize(FVector2D(100.f, 30.f));
		StatusEffectWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void APC_BaseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

float APC_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser)
{
	if(IPC_PlayerCharacterInterface* PlayerCharacter = Cast<IPC_PlayerCharacterInterface>(this))
	{
		 if(UPC_ActionComponent* ActionComponent = PlayerCharacter->GetActionComponent())
		 {
			 if(ActionComponent->IsGuarded())
			 {
			 	return 0;
			 }
		 }
	}
	
	const float VariancePercent = 0.035f;
	const float RandomFactor = FMath::FRandRange(-VariancePercent, VariancePercent);
	const float FinalDamage = DamageAmount * (1.0f + RandomFactor);

	const float Damage = StatComponent->ApplyDamage(FinalDamage, DamageCauser, false);
	if (Damage > KINDA_SMALL_NUMBER)
	{
		if (DamageEvent.IsOfType(FNormalAttackDamageEvent::ClassID))
		{
			//노말머테리얼일때만
			FPC_GameUtil::PlayHitMaterial(this);
		}
	}

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void APC_BaseCharacter::ApplyStat(const FPC_CharacterStatTableRow& BaseStat,
                                  const FPC_CharacterStatTableRow& ModifierStat)
{
	float MovementSpeed = (BaseStat + ModifierStat).MovementSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void APC_BaseCharacter::SetupCharacterWidget(UPC_UserWidget* InWidget)
{
	//액터 생성과 동시에 바로 보여줘야하는 위젯들은 여기서 ㄱㄱ
	//if (UPC_HPBarWidget* HPBarWidget = Cast<UPC_HPBarWidget>(InWidget))
	//{
	//	StatComponent->OnHPChangedDelegate.AddUObject(HPBarWidget, &UPC_HPBarWidget::UpdateHpBar);
	//	HPBarWidget->UpdateHpBar(StatComponent->GetCurrentHp(), StatComponent->GetMaxHp());
	//	HPBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	//}
}

void APC_BaseCharacter::SetupIndicatorWidget(UPC_UserWidget* InUserWidget)
{
	if (UPC_IndicatorWidget* IndicatorWidget = Cast<UPC_IndicatorWidget>(InUserWidget))
	{
		OnCharacterSelectedAssassinateTarget.AddDynamic(IndicatorWidget, &UPC_IndicatorWidget::ToggleAssassinateImage);
		OnCharacterLocked.AddDynamic(IndicatorWidget, &UPC_IndicatorWidget::ToggleLockOnImage);
	}
}

void APC_BaseCharacter::OnLocked(bool bLocked)
{
	OnCharacterLocked.Broadcast(bLocked);
}

void APC_BaseCharacter::SetupStatusEffectWidget(UPC_UserWidget* InUserWidget)
{
	if (UPC_StatusEffectWidget* StatusEffectWidget = Cast<UPC_StatusEffectWidget>(InUserWidget))
	{
		OnCharacterApplyStatusEffect.AddDynamic(StatusEffectComponent, &UPC_StatusEffectComponent::ApplyStatusEffect);
		StatusEffectComponent->OnStatusEffectTimeUpdate.AddUObject(StatusEffectWidget,
		                                                           &UPC_StatusEffectWidget::UpdateStatusEffect);
	}
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

void APC_BaseCharacter::OnSelectedAssassinateTarget(bool bSelected)
{
	OnCharacterSelectedAssassinateTarget.Broadcast(bSelected);
}

void APC_BaseCharacter::OnApplyStatusEffect(uint32 StatusEffectId)
{
	OnCharacterApplyStatusEffect.Broadcast(StatusEffectId);
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

void APC_BaseCharacter::ReactAttackBreak()
{
	//TODO 공통 Cam shake 넣어도 될듯
}

void APC_BaseCharacter::WeaponSparkEffect(bool bStart, bool bRight)
{
	if(BattleComponent)
	{
		BattleComponent->ShowWeaponSparkEffect(bStart, bRight);
	}
}

bool APC_BaseCharacter::IsDead()
{
	check(StatComponent);
	return StatComponent->CurrentHp < KINDA_SMALL_NUMBER;
}

bool APC_BaseCharacter::IsRolling()
{
	return false;
}

bool APC_BaseCharacter::IsGuarding(FVector ImpactPoint)
{
	return false;
}

TPair<FName, FName> APC_BaseCharacter::GetWeaponTraceNames(bool bRight)
{
	return {BattleComponent->TraceStartBoneName, BattleComponent->TraceEndBoneName};
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

void APC_BaseCharacter::OnStartSkill(uint32 SkillId)
{
	
}

void APC_BaseCharacter::OnEndSkill(uint32 SkillId)
{
	
}

void APC_BaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StatComponent->OnCharacterDieDelegate.AddUObject(this, &ThisClass::OnDead);
	StatComponent->OnStatChangedDelegate.AddUObject(this, &ThisClass::ApplyStat);

	//StatusEffectComponent->OnStatusEffectTimeUpdate.AddUObject(this, &ThisClass::OnApplyStatusEffect);

	if(!SkillComponent->OnStartSkillDelegate.IsAlreadyBound(this, &ThisClass::OnStartSkill))
		SkillComponent->OnStartSkillDelegate.AddDynamic(this, &ThisClass::OnStartSkill);

	if(!SkillComponent->OnEndSkillDelegate.IsAlreadyBound(this, &ThisClass::OnEndSkill))
		SkillComponent->OnEndSkillDelegate.AddDynamic(this, &ThisClass::OnEndSkill);
}

void APC_BaseCharacter::AttackTrace(bool bStart, FName TraceStartBoneName, FName TraceEndBoneName)
{
	check(BattleComponent);

	if (bStart)
		BattleComponent->StartTrace(TraceStartBoneName, TraceEndBoneName);
	else
		BattleComponent->EndTrace();
}

void APC_BaseCharacter::AttackTraceWithWeapon(bool bStart, bool bRight, bool PowerAttack)
{
	if (bStart)
		BattleComponent->StartTraceWithWeapon(bRight,PowerAttack);
	else
		BattleComponent->EndTrace();
}

bool APC_BaseCharacter::HasWeapon()
{
	check(BattleComponent);
	return BattleComponent->HasWeapon();
}

//
