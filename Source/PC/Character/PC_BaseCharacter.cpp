// Copyright Epic Games, Inc. All Rights Reserved.

#include "PC_BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Component/PC_StatComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "Component/PC_BattleComponent.h"
#include "PC/PC.h"
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
	StatComponent = CreateDefaultSubobject<UPC_StatComponent>(TEXT("StatComponent"));
	WeaponStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticComponent"));
	WeaponStaticMeshComponent->SetupAttachment(GetMesh());
	
	WidgetComponent = CreateDefaultSubobject<UPC_WidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetMesh());
	

	//TODO 데이터 분리 필요
	static ConstructorHelpers::FClassFinder<UUserWidget> HpBarWidgetRef(TEXT("/Game/ProjectClass/UI/WBP_HPBar.WBP_HPBar_C"));
	if (HpBarWidgetRef.Class)
	{
		WidgetComponent->SetWidgetClass(HpBarWidgetRef.Class);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(150.0f, 15.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
		
	UE_LOG(LogPC, Log, TEXT(" Constructor"));
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void APC_BaseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

float APC_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	StatComponent->ApplyDamage(DamageAmount);

	FPC_GameUtil::CameraShake();

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void APC_BaseCharacter::ApplyStat(const FPC_CharacterStatTableRow& BaseStat, const FPC_CharacterStatTableRow& ModifierStat)
{
	float MovementSpeed = (BaseStat + ModifierStat).MovementSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void APC_BaseCharacter::SetupCharacterWidget(UPC_UserWidget* InWidget)
{
	if (UPC_HPBarWidget* HpBarWidget = Cast<UPC_HPBarWidget>(InWidget))
	{
		HpBarWidget->UpdateHpBar(StatComponent->GetCurrentHp(), StatComponent->GetMaxHp());
		StatComponent->OnHPChangedDelegate.AddUObject(HpBarWidget, &UPC_HPBarWidget::UpdateHpBar);
	}
}

void APC_BaseCharacter::SetupLockOnWidget(UPC_UserWidget* InUserWidget)
{
	if (UPC_LockOnWidget* LockOnWidget = Cast<UPC_LockOnWidget>(InUserWidget))
	{
		OnCharacterLocked.AddDynamic(LockOnWidget, &UPC_LockOnWidget::ToggleActivation);
	}
}

void APC_BaseCharacter::OnLocked(bool bLocked)
{
	OnCharacterLocked.Broadcast(bLocked);
}

void APC_BaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
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

void APC_BaseCharacter::AttackTraceWithWeapon(bool bStart)
{
	if (bStart)
		BattleComponent->StartTraceWithWeapon();
	else
		BattleComponent->EndTrace();
}

bool APC_BaseCharacter::HasWeapon()
{
	check(BattleComponent);
	return BattleComponent->HasWeapon();
}
//



