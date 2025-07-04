﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_PlayableCharaceter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Component/PC_ActionComponent.h"
#include "Component/PC_AimComponent.h"
#include "Component/PC_BattleComponent.h"
#include "Component/PC_LockOnComponent.h"
#include "Component/PC_SkillComponent.h"
#include "Component/PC_StatComponent.h"
#include "Component/PC_WidgetComponent.h"
#include "Controller/PC_PlayerController.h"
#include "Core/Tests/Containers/TestUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PC/Data/PC_InputDataAsset.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/UI/PC_HUDWidget.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"


// Sets default values
APC_PlayableCharaceter::APC_PlayableCharaceter()
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true;
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	StimulusSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulus"));
	StimulusSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
	StimulusSource->RegisterWithPerceptionSystem();

	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	LockOnComponent = CreateDefaultSubobject<UPC_LockOnComponent>(TEXT("LockOnComponent"));
	ActionComponent = CreateDefaultSubobject<UPC_ActionComponent>(TEXT("ActionComponent"));
	AimComponent = CreateDefaultSubobject<UPC_AimComponent>(TEXT("AimComponent"));
	SkillComponent = CreateDefaultSubobject<UPC_SkillComponent>(TEXT("SkillComponent"));
}

void APC_PlayableCharaceter::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	check(BattleComponent);
	BattleComponent->EquipWeapon(0);
}

void APC_PlayableCharaceter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UE_LOG(LogTemp, Log, TEXT(" SetupPlayerInputComponent"));
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(InputData->JumpAction, ETriggerEvent::Triggered, this, &APC_PlayableCharaceter::Jump);
		EnhancedInputComponent->BindAction(InputData->JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(InputData->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(InputData->LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(InputData->AttackAction, ETriggerEvent::Triggered, this, &ThisClass::Attack);

		EnhancedInputComponent->BindAction(InputData->SpecialAction, ETriggerEvent::Triggered, this, &ThisClass::SpecialAction);
		EnhancedInputComponent->BindAction(InputData->LockOnAction, ETriggerEvent::Triggered, this, &ThisClass::LockOn);
		EnhancedInputComponent->BindAction(InputData->RunAction, ETriggerEvent::Triggered, this, &ThisClass::Run);
		EnhancedInputComponent->BindAction(InputData->RollAction, ETriggerEvent::Triggered, this, &ThisClass::Roll);
		//
		EnhancedInputComponent->BindAction(InputData->WeaponSwapAction, ETriggerEvent::Triggered, this, &ThisClass::WeaponSwap);
		EnhancedInputComponent->BindAction(InputData->Num1Action, ETriggerEvent::Triggered, this, &ThisClass::Num1);
	}
}

void APC_PlayableCharaceter::Move(const FInputActionValue& Value)
{
	FVector2D Movement = Value.Get<FVector2D>();
	
	check(ActionComponent);
	ActionComponent->Move(Movement);
}

void APC_PlayableCharaceter::Jump(const FInputActionValue& Value)
{
	Super::Jump();

	MakeNoise(1, this, GetActorLocation());
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), PlayerData->JumpSound, GetActorLocation());
}

void APC_PlayableCharaceter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if(!LockOnComponent->IsLockOnMode())
		{
			// add yaw and pitch input to controller
			AddControllerYawInput(LookAxisVector.X);
			AddControllerPitchInput(LookAxisVector.Y);	
		}
	}
}


void APC_PlayableCharaceter::Attack(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	
	check(ActionComponent);
	check(BattleComponent)

	if(BattleComponent->CharacterStanceType == EPC_CharacterStanceType::Staff
		&& ActionComponent->IsInSpecialAction)
	{
		BattleComponent->FireProjectile(IsPressed);
	}
	else
	{
		ActionComponent->Attack(IsPressed);
	}
	
}


void APC_PlayableCharaceter::SpecialAction(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;

	check(ActionComponent);
	AdjustMovement(IsPressed);
	AdjustCamera(IsPressed);
	ActionComponent->SpecialAction(IsPressed);
}

void APC_PlayableCharaceter::Run(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	
	check(ActionComponent);
	ActionComponent->Run(IsPressed);
}

void APC_PlayableCharaceter::Roll(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;

	check(ActionComponent);
	ActionComponent->Roll(IsPressed);
}

void APC_PlayableCharaceter::WeaponSwap(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;

	check(ActionComponent);
	ActionComponent->SwapWeapon(IsPressed);
}


void APC_PlayableCharaceter::LockOn(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;

	check(LockOnComponent);
	LockOnComponent->LockOn();
}

void APC_PlayableCharaceter::Num1(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;

	check(SkillComponent);
	SkillComponent->RequestPlaySkill(*PlayerData->SkillIds.Find(EPC_SkillSlotType::Num_1));
}

void APC_PlayableCharaceter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController->GetClass() == APC_PlayerController::StaticClass())
		SetGenericTeamId(FGenericTeamId(0));
}

void APC_PlayableCharaceter::SetupHUDWidget(UPC_HUDWidget* InWidget)
{
	if (InWidget)
	{
		InWidget->UpdateStat(StatComponent->GetBaseStat(), StatComponent->GetModifierStat());
		StatComponent->OnStatChangedDelegate.AddUObject(InWidget, &UPC_HUDWidget::UpdateStat);
	}
}

//bOrientRotationToMovement : true 가속을 받는 방향으로 캐릭터가 회전
void APC_PlayableCharaceter::AdjustMovement(bool IsPressed)
{
	if (IsPressed && !ActionComponent->IsInSpecialAction)
	{
		GetCharacterMovement()->MaxWalkSpeed = PlayerData->MovementSpeed_Walk;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else if (!IsPressed && ActionComponent->IsInSpecialAction)
	{
		GetCharacterMovement()->MaxWalkSpeed = PlayerData->MovementSpeed_Jog;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void APC_PlayableCharaceter::AdjustCamera(bool bIsPressed)
{
	if (bIsPressed && !ActionComponent->IsInSpecialAction)
	{
		if (BattleComponent->CharacterStanceType == EPC_CharacterStanceType::Staff && AimComponent->CurrentCameraType != EPC_CameraType::Aim)
		{
			AimComponent->SwitchCamera(EPC_CameraType::Aim);
		}
	}
	else if (!bIsPressed && ActionComponent->IsInSpecialAction)
	{
		if (BattleComponent->CharacterStanceType == EPC_CharacterStanceType::Staff && AimComponent->CurrentCameraType != EPC_CameraType::Normal)
		{
			AimComponent->SwitchCamera(EPC_CameraType::Normal);
		}
	}
}

void APC_PlayableCharaceter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	GenericTeamId = TeamID;
}

FGenericTeamId APC_PlayableCharaceter::GetGenericTeamId() const
{
	return GenericTeamId;
}