// Fill out your copyright notice in the Description page of Project Settings.


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
//#include "Core/Tests/Containers/TestUtils.h"
#include <string>

#include "Engine/DamageEvents.h"
#include "Exporters/TextureExporterPNG.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PC/Battle/PC_NormalAttackDamageType.h"
#include "PC/Data/PC_InputDataAsset.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/Misc/GameMode/PCGameMode.h"
#include "PC/Subsystem/PC_UISubsystem.h"
#include "PC/UI/PC_HUDWidget.h"
#include "PC/Utills/PC_GameUtill.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"


// Sets default values
APC_PlayableCharaceter::APC_PlayableCharaceter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f; //데이터로 제어함	
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

	InteractionComponent = CreateDefaultSubobject<UPC_InteractionComponent>(TEXT("InteractionComponent"));
	InteractionOverlapComponent = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionOverlapComponent"));
	InteractionOverlapComponent->SetupAttachment(RootComponent);

	ArcSplinePreviewComponent = CreateDefaultSubobject<UPC_ArcSplinePreviewComponent>(TEXT("ArcSplinePreviewComponent"));
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

	if(!InteractionOverlapComponent->OnComponentBeginOverlap.IsAlreadyBound(InteractionComponent.Get(), &UPC_InteractionComponent::OnBeginOverlap))
		InteractionOverlapComponent->OnComponentBeginOverlap.AddDynamic(InteractionComponent.Get(), &UPC_InteractionComponent::OnBeginOverlap);

	if(!InteractionOverlapComponent->OnComponentEndOverlap.IsAlreadyBound(InteractionComponent.Get(), &UPC_InteractionComponent::OnEndOverlap))
		InteractionOverlapComponent->OnComponentEndOverlap.AddDynamic(InteractionComponent.Get(), &UPC_InteractionComponent::OnEndOverlap);


	InitPPFromGameMode();

	// 커브가 세팅되어 있으면 타임라인 초기화
	if (PPBlurCurve)
	{
		FOnTimelineFloat UpdateCallback;
		UpdateCallback.BindUFunction(this, FName("OnPPBlurUpdate"));

		FOnTimelineEvent FinishedCallback;
		FinishedCallback.BindUFunction(this, FName("OnPPBlurFinished"));

		PPBlurTimeline.AddInterpFloat(PPBlurCurve, UpdateCallback);
		PPBlurTimeline.SetTimelineFinishedFunc(FinishedCallback);
		PPBlurTimeline.SetLooping(false);
	}
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
		EnhancedInputComponent->BindAction(InputData->BackstabOnAction, ETriggerEvent::Triggered, this, &ThisClass::Assassinate);
		
		EnhancedInputComponent->BindAction(InputData->RunAction, ETriggerEvent::Triggered, this, &ThisClass::Run);
		EnhancedInputComponent->BindAction(InputData->RollAction, ETriggerEvent::Triggered, this, &ThisClass::Roll);
		//
		EnhancedInputComponent->BindAction(InputData->WeaponSwapAction, ETriggerEvent::Triggered, this, &ThisClass::WeaponSwap);
		EnhancedInputComponent->BindAction(InputData->Num1Action, ETriggerEvent::Triggered, this, &ThisClass::Num1);
		EnhancedInputComponent->BindAction(InputData->Num2Action, ETriggerEvent::Triggered, this, &ThisClass::Num2);
		EnhancedInputComponent->BindAction(InputData->Num3Action, ETriggerEvent::Triggered, this, &ThisClass::Num3);
		EnhancedInputComponent->BindAction(InputData->Num4Action, ETriggerEvent::Triggered, this, &ThisClass::Num4);

		//
		EnhancedInputComponent->BindAction(InputData->Num5Action, ETriggerEvent::Ongoing, this, &ThisClass::Num5Ongoing);
		EnhancedInputComponent->BindAction(InputData->Num5Action, ETriggerEvent::Started, this, &ThisClass::Num5Started);
		EnhancedInputComponent->BindAction(InputData->Num5Action, ETriggerEvent::Completed, this, &APC_PlayableCharaceter::Num5Released);
		EnhancedInputComponent->BindAction(InputData->Num5Action, ETriggerEvent::Canceled, this, &ThisClass::Num5Canceled);

		//
		EnhancedInputComponent->BindAction(InputData->OpenOptionAction, ETriggerEvent::Triggered, this, &ThisClass::OpenOptionSetting);

		//Debug
		EnhancedInputComponent->BindAction(InputData->DebugDrawAction, ETriggerEvent::Triggered, this, &ThisClass::DebugDraw);

	}
}

void APC_PlayableCharaceter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PPBlurTimeline.TickTimeline(DeltaTime);
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
	const bool IsPressed = Value[0] != 0.f;
	
	check(ActionComponent);
	ActionComponent->Jump(IsPressed);
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
			AddControllerYawInput(LookAxisVector.X * MouseSensitivity);
			AddControllerPitchInput(LookAxisVector.Y * MouseSensitivity);	
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

void APC_PlayableCharaceter::Assassinate(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;
	
	//

	check(ActionComponent);
	ActionComponent->Assassinate(IsPressed);
}

void APC_PlayableCharaceter::Num1(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;
	
	check(SkillComponent);
	check(BattleComponent);
	check(ActionComponent);
	
	const uint32 SkillId = FPC_GameUtil::GetSkillId(PlayerData,
		EPC_SkillSlotType::Num_1,
		BattleComponent->CharacterStanceType,
		ActionComponent->IsInSpecialAction);
	
	SkillComponent->RequestPlaySkill(SkillId);
}

void APC_PlayableCharaceter::Num2(const FInputActionValue& Value)
{	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;
	
	check(SkillComponent);
	check(BattleComponent);
	check(ActionComponent);
	
	const uint32 SkillId = FPC_GameUtil::GetSkillId(PlayerData,
		EPC_SkillSlotType::Num_2,
		BattleComponent->CharacterStanceType,
		ActionComponent->IsInSpecialAction);
	
	SkillComponent->RequestPlaySkill(SkillId);
}

void APC_PlayableCharaceter::Num3(const FInputActionValue& Value)
{	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;
	
	check(SkillComponent);
	check(BattleComponent);
	check(ActionComponent);
	
	const uint32 SkillId = FPC_GameUtil::GetSkillId(PlayerData,
		EPC_SkillSlotType::Num_3,
		BattleComponent->CharacterStanceType,
		ActionComponent->IsInSpecialAction);
	
	SkillComponent->RequestPlaySkill(SkillId);
}

void APC_PlayableCharaceter::Num4(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;
	
	check(SkillComponent);
	check(BattleComponent);
	check(ActionComponent);
	
	const uint32 SkillId = FPC_GameUtil::GetSkillId(PlayerData,
		EPC_SkillSlotType::Num_4,
		BattleComponent->CharacterStanceType,
		ActionComponent->IsInSpecialAction);
	
	SkillComponent->RequestPlaySkill(SkillId);
}

void APC_PlayableCharaceter::Num5Ongoing(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (IsPressed)
		return;

	const uint32 SkillId = FPC_GameUtil::GetSkillId(PlayerData,
	EPC_SkillSlotType::Num_5,
	BattleComponent->CharacterStanceType,
	ActionComponent->IsInSpecialAction);
	
	if(SkillId > 0)
	{
		check(ArcSplinePreviewComponent);
		
		USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
		check(SkeletalMeshComponent);

		FVector Location = SkeletalMeshComponent->GetSocketLocation(TEXT("hand_l"));
		FRotator Rotation = GetControlRotation();
		
		// 시작점을 카메라 앞쪽으로 약간 빼서 자기 몸/벽과의 충돌을 피함
		const FVector Forward = Rotation.Vector();
		const float   ForwardOffset = 60.f;   // 필요 시 조정
		const float   UpOffset      = 5;
		const FVector StartPos = Location + Forward * ForwardOffset + FVector(0,0,UpOffset);
		const float   Speed    = 2200.f; // 프리뷰 전용

		const float ArcUpBias = 0.35f; // 0.0 ~ 0.6 정도에서 조절
		FVector Dir = Forward + ArcUpBias * FVector::UpVector;
		const FVector StartVel = Dir * Speed;

		ArcSplinePreviewComponent->UpdateFromStartVelocity(StartPos, StartVel);
	}
}

void APC_PlayableCharaceter::Num5Started(const FInputActionValue& Value)
{
	FPC_GameUtil::AddOnScreenDebugMessage(TEXT("Num5Started"));
	
	check(ArcSplinePreviewComponent);
	ArcSplinePreviewComponent->BeginPreview();
	ArcSplinePreviewComponent->AddActorToIgnore(this);
}

void APC_PlayableCharaceter::Num5Released(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (IsPressed)
		return;
	
	const uint32 SkillId = FPC_GameUtil::GetSkillId(PlayerData,
	EPC_SkillSlotType::Num_5,
	BattleComponent->CharacterStanceType,
	ActionComponent->IsInSpecialAction);
	
	SkillComponent->RequestPlaySkill(SkillId);
	check(ArcSplinePreviewComponent);
	ArcSplinePreviewComponent->EndPreview();
}

void APC_PlayableCharaceter::Num5Canceled(const FInputActionValue& Value)
{
}

void APC_PlayableCharaceter::OpenOptionSetting(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.f;
	if (!IsPressed)
		return;

	if (GEngine)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_UISubsystem* UISubsystem = GameInstance->GetSubsystem<UPC_UISubsystem>())
			{
				UISubsystem->ToggleOptionsWidget();
			}
		}
	}
}


void APC_PlayableCharaceter::DebugDraw(const FInputActionValue& Value)
{
	const bool IsPressed = Value[0] != 0.0f;
	if(!IsPressed)
	{
		return;
	}

	UWorld* World = GetWorld();
	check(World);

	APCGameMode* GameMode = Cast<APCGameMode>(World->GetAuthGameMode());
	check(GameMode);

	GameMode->DebugDrawing = !GameMode->DebugDrawing;
}

float APC_PlayableCharaceter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	IPC_CharacterInterface* OwnerCharacterInterface =  Cast<IPC_CharacterInterface>(this);
	check(OwnerCharacterInterface);
	
	IPC_CharacterInterface* CauserInterface =  Cast<IPC_CharacterInterface>(DamageCauser);
	check(CauserInterface);

	UPC_CharacterDataAsset* OwnerDataAsset = OwnerCharacterInterface->GetCharacterDataAsset();
	check(OwnerDataAsset);

	UPC_CharacterDataAsset* CauserDataAsset = CauserInterface->GetCharacterDataAsset();
	check(CauserDataAsset);

	if(ActionComponent->IsGuarded())
	{
		return 0;
	}
	
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if(Damage > KINDA_SMALL_NUMBER)
	{
		if (DamageEvent.IsOfType(FNormalAttackDamageEvent::ClassID))
		{
			CrowdControlComponent->RequestPlayerCC(4, DamageCauser);
		}

		FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), OwnerDataAsset->HitFx, GetActorLocation(), FRotator::ZeroRotator);
	}
	else
	{
		if(ActionComponent->IsInSpecialAction &&
			BattleComponent->CharacterStanceType == EPC_CharacterStanceType::Sword)
		{
			FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), OwnerDataAsset->HitFx, GetActorLocation(), FRotator::ZeroRotator);
		}
	}
	
	return Damage;
}

void APC_PlayableCharaceter::OnSensedByBossMonster(ACharacter* Incharacter) const
{
	OnEnCounterBossMonsterDelegate.Broadcast(Incharacter);

	if(Incharacter)
	{
		if (UWorld* World = GetWorld())
		{
			if (APCGameMode* GM = World->GetAuthGameMode<APCGameMode>())
			{
				GM->PlayBGM(EPC_BGMType::Combat);
			}
		}
	}
}

void APC_PlayableCharaceter::OnPPBlurUpdate(float Value)
{
	if (CombatPPMID)
	{
		CombatPPMID->SetScalarParameterValue(TEXT("EffectIntensity"), Value);
	}
}

void APC_PlayableCharaceter::OnPPBlurFinished()
{
	if (CombatPPMID)
	{
		CombatPPMID->SetScalarParameterValue(TEXT("EffectIntensity"), 0.0f);
	}
}

void APC_PlayableCharaceter::InitPPFromGameMode()
{
	if (UWorld* World = GetWorld())
	{
		if (APCGameMode* GM = World->GetAuthGameMode<APCGameMode>())
		{
			CombatPPVolume = GM->GetCombatPPVolume();
			CombatPPMID = GM->GetCombatPPMID();
		}
	}
}

void APC_PlayableCharaceter::PlayHitBlurEffect()
{
	if (!PPBlurCurve || !CombatPPMID)
		return;
	
	if(FPC_GameUtil::IsDebugDrawing(this))
		return;
	
	PPBlurTimeline.PlayFromStart();
}

void APC_PlayableCharaceter::PlayHitBlurEffect(const FVector& ImpactPointWorld, const FVector& HitFromWorldDir)
{
	if (!PPBlurCurve || !CombatPPMID)
		return;

	if(FPC_GameUtil::IsDebugDrawing(this))
		return;
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	
	FVector2D ScreenPos;
	if (!PC->ProjectWorldLocationToScreen(ImpactPointWorld, ScreenPos, true))
		return;

	int32 ViewX = 0, ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX <= 0 || ViewY <= 0)
		return;

	const FVector2D ViewSize(ViewX, ViewY);
	FVector2D ImpactUV = ScreenPos / ViewSize;

	// Y 플립 테스트
	//ImpactUV.Y = 1.f - ImpactUV.Y;

	ImpactUV.X = FMath::Clamp(ImpactUV.X, 0.f, 1.f);
	ImpactUV.Y = FMath::Clamp(ImpactUV.Y, 0.f, 1.f);

	CombatPPMID->SetVectorParameterValue(
		TEXT("BlurCenter"),
		FLinearColor(ImpactUV.X, ImpactUV.Y, 0.f, 0.f));
	
	PPBlurTimeline.PlayFromStart();
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
		InWidget->SetOwningActor(this);
		
		StatComponent->OnStatChangedDelegate.AddUObject(InWidget, &UPC_HUDWidget::UpdateStat);
		
		StatComponent->OnHPChangedDelegate.AddUObject(InWidget, &UPC_HUDWidget::UpdateHPBar);
		StatComponent->OnMPChangedDelegate.AddUObject(InWidget, &UPC_HUDWidget::UpdateMPBar);
		StatComponent->OnStaminaChangedDelegate.AddUObject(InWidget, &UPC_HUDWidget::UpdateStaminaBar);

		OnEnCounterBossMonsterDelegate.AddUObject(InWidget, &UPC_HUDWidget::OnEnCounterBossMonster);
	}
}

void APC_PlayableCharaceter::ReactAttackBreak()
{
	
}

void APC_PlayableCharaceter::WeaponSparkEffect(bool bStart, bool bRight)
{
	Super::WeaponSparkEffect(bStart, bRight);
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

void APC_PlayableCharaceter::PlayCameraAnim(EPC_CameraType CameraType, float Time)
{
	check(AimComponent);
	AimComponent->PlayCameraAnim(CameraType, Time);
}

void APC_PlayableCharaceter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	GenericTeamId = TeamID;
}

FGenericTeamId APC_PlayableCharaceter::GetGenericTeamId() const
{
	return GenericTeamId;
}

bool APC_PlayableCharaceter::IsGuarding(FVector ImpactPoint)
{
	check(ActionComponent);
	bool bIsGuarding = ActionComponent->IsInSpecialAction;
	
	if(bIsGuarding)
	{
		LaunchCharacter(GetActorLocation(), ImpactPoint, 20);
	
		if (UPC_CharacterDataAsset* HitCharDataAsset = GetCharacterDataAsset())
		{
			FPC_GameUtil::SpawnEffectAtLocation(this,
				HitCharDataAsset->GuardFx, ImpactPoint,
				FRotator::ZeroRotator, 1);

			if(HitCharDataAsset->HitGuardAnimMontage)
			{
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				check(AnimInstance);

				ActionComponent->ResetCombo();
				AnimInstance->StopAllMontages(0.f);
				AnimInstance->Montage_Play(HitCharDataAsset->HitGuardAnimMontage, 1.f, EMontagePlayReturnType::MontageLength);
				FOnMontageEnded EndDelegate;
				EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
				{
					if (!IsDead())
					{
					
					}
				});
				AnimInstance->Montage_SetEndDelegate(EndDelegate, HitCharDataAsset->HitGuardAnimMontage);
			}
		}
	}

	return bIsGuarding;
}

bool APC_PlayableCharaceter::IsRolling()
{
	check(ActionComponent);
	
	return ActionComponent->IsRolling;
}
