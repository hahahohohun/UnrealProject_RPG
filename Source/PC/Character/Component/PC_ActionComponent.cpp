// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_ActionComponent.h"

#include <string>

#include "AnalyticsProviderETEventCache.h"
#include "PC_BattleComponent.h"
#include "PC_LockOnComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PC/Character/PC_PlayableCharaceter.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/SkillObject/PC_SkillObject.h"
#include "Components/ArrowComponent.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_ActionComponent::UPC_ActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPC_ActionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
	
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	check(Interface);
	
	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	TArray<UAnimMontage*>& AttackMontages = PlayerData->AttackMontages;
	AttackMaxCount = AttackMontages.Num();
	
	if (ACharacter* C = Cast<ACharacter>(GetOwner()))
	{
		DebugInputArrow = NewObject<UArrowComponent>(C, TEXT("InputDirArrow"));
		DebugInputArrow->SetupAttachment(C->GetRootComponent());
		DebugInputArrow->RegisterComponent();

		DebugInputArrow->ArrowSize = 1.f;              // 작게
		DebugInputArrow->SetHiddenInGame(true);
		DebugInputArrow->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	}
}

void UPC_ActionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//가속을 주고 있으면 내가 바라보고 있는 방향으로 회전
	if (IsInSpecialAction && !OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero())
	{
		FRotator LookAtRot = OwnerCharacter->GetBaseAimRotation();
		LookAtRot.Pitch = 0.f; // 평면 회전만

		const FRotator CurrentRot = OwnerCharacter->GetActorRotation();
		const FRotator NewRot = FMath::RInterpTo(CurrentRot, LookAtRot, GetWorld()->GetDeltaSeconds(), 10.f);

		OwnerCharacter->SetActorRotation(NewRot);
	}

	Tick_Running(DeltaTime);

	if (DebugInputArrow)
	{
		// 입력 없으면 숨김
		if (InputVector.IsNearlyZero() || !FPC_GameUtil::IsDebugDrawing(this))
		{
			DebugInputArrow->SetHiddenInGame(true);
			return;
		}

		DebugInputArrow->SetHiddenInGame(false);

		const FRotator ControlRot = OwnerCharacter->GetControlRotation();
		const FRotator YawOnly(0.f, ControlRot.Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);
		const FVector TargetWorldDir = (Right * InputVector.X + Forward * InputVector.Y).GetSafeNormal();
	
		const float SmoothSpeed = 10.f;
		DebugDirSmoothed = FMath::VInterpTo(DebugDirSmoothed, TargetWorldDir, DeltaTime, SmoothSpeed).GetSafeNormal();

		const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
		const FName PelvisSocket(TEXT("pelvis")); // 허리 본 이름
		FVector Start = OwnerCharacter->GetActorLocation();

		if (Mesh && Mesh->DoesSocketExist(PelvisSocket))
		{
			Start = Mesh->GetSocketLocation(PelvisSocket);
		}
		else
		{
			// 허리쯤 높이로 보정 (대략 +80cm)
			Start += FVector(0.f, 0.f, 80.f);
		}

		// 4️⃣ 크기 및 회전 조정
		const float ArrowLen   = 160.f;             // 짧게
		const float BaseLen    = 100.f;
		const float ScaleX     = ArrowLen / BaseLen;
		const FRotator DirRot  = DebugDirSmoothed.Rotation();

		DebugInputArrow->SetWorldLocation(Start);
		DebugInputArrow->SetWorldRotation(DirRot);
		//DebugInputArrow->SetWorldScale3D(FVector(ScaleX, 0.3f, 0.3f)); // 얇고 슬림하게

		DrawFeetSpheres(OwnerCharacter, /*Radius=*/8.f, /*Life=*/GetWorld()->GetDeltaSeconds() * 1.5f);
	}

}

void UPC_ActionComponent::Tick_Running(float DeltaTime)
{
	if(IsRunning)
	{
		const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
		UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
		check(PlayerData);

		const FPC_ActionStaminaData* ActionStaminaData = GetActionStaminaData(EPC_ActionType::Run);
		check(ActionStaminaData);

		if(ActionStaminaData->MaintainCostPerSec > 0.f)
		{
			const IPC_CharacterInterface* Character = CastChecked<IPC_CharacterInterface>(GetOwner());
			UPC_StatComponent* StatComponent = Character->GetStatComponent();
			check(StatComponent);

			const float Cost = ActionStaminaData->MaintainCostPerSec * DeltaTime;
			if(!StatComponent->TryConsumeStamina(Cost))
			{
				Run(false);
			}
		}
	}

}
void UPC_ActionComponent::Move(FVector2D MovementVector)
{
	if (!CanAction(EPC_ActionType::Move))
		return;
	
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	const UPC_LockOnComponent* LockOnComponent = Interface->GetLockOnComponent();
	check(LockOnComponent);
	
	InputVector = MovementVector;
	
	if (!LockOnComponent->IsLockOnMode() || IsRunning)
	{
		ProcessFreeMove();
	}
	else
	{
		ProcessLockOnMove();
	}
	
	if (IsAttacking)
	{
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.1f);
			ResetCombo();
		}
	}
}

void UPC_ActionComponent::Jump(bool IsPressed)
{
	if (!IsPressed)
		return;
	
	if (!CanAction(EPC_ActionType::Jump))
		return;

	if(!TryConsumeStaminaOnActionStart(EPC_ActionType::Jump))
		return;
	
	OwnerCharacter->Jump();

	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	const UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	OwnerCharacter->MakeNoise(1, OwnerCharacter, OwnerCharacter->GetActorLocation());
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), PlayerData->JumpSound, OwnerCharacter->GetActorLocation());
}

void UPC_ActionComponent::Attack(bool IsPressed)
{
	if (!IsPressed)
		return;

	if (!CanAction(EPC_ActionType::Attack))
		return;

	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	float MaxAcceleration = MoveComp->MaxAcceleration;

	// 현재 가속의 크기 (length)
	float CurrentSpeed = MoveComp->Velocity.Size();
	bool IsMaxSpeed = false;// 안씀 PlayerData->RunAttackMontage && CurrentSpeed >= PlayerData->MovementSpeed_Sprint;
	
	TArray<UAnimMontage*>& AttackMontages = PlayerData->AttackMontages;
	if(AttackMaxCount == 0)
		AttackMaxCount = AttackMontages.Num();

	if (IsAttacking && IsMaxSpeed == false)
	{
		SaveAttack = true;
	}
	else
	{
		if(!TryConsumeStaminaOnActionStart(EPC_ActionType::Attack))
			return;
		
		IsAttacking = true;
		AttackCount++;
		
		if (AttackCount > AttackMaxCount)
		{
			AttackCount = 0;
		}

		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		check(AnimInstance);

		UAnimMontage* AttackMontage = nullptr;
		if(IsMaxSpeed)
		{
			AttackMontage = PlayerData->RunAttackMontage;
		}
		else
		{
			if (AttackMontages.IsValidIndex(AttackCount - 1))
			{
				AttackMontage = AttackMontages[AttackCount - 1];
			}
		}

		if(AttackMontage)
			OwnerCharacter->PlayAnimMontage(AttackMontage);
		
		RotateToControlRotation();

		AddLock(EPC_LockCauseType::Attack, EPC_ActionType::Move);
		AddLock(EPC_LockCauseType::Attack, EPC_ActionType::Jump);
		AddLock(EPC_LockCauseType::Attack, EPC_ActionType::Guard);
	}
}
//Guard or Aim
void UPC_ActionComponent::SpecialAction(bool bPressed)
{
	if (bPressed && !IsInSpecialAction)
	{
		if (!CanAction(EPC_ActionType::Guard))
			return;
		
		IsInSpecialAction = true;

		AddLock(EPC_LockCauseType::SpecialAction, EPC_ActionType::Run);
	}
	else if (!bPressed && IsInSpecialAction)
	{
		IsInSpecialAction = false;

		ForceReleaseLock(EPC_LockCauseType::SpecialAction);
	}
}

void UPC_ActionComponent::Run(bool bPressed)
{
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);
	
	if (bPressed && !IsRunning)
	{
		if (!CanAction(EPC_ActionType::Run))
			return;
		
		IsRunning = true;

		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = PlayerData->MovementSpeed_Sprint;
	}
	else if (!bPressed && IsRunning)
	{
		IsRunning = false;
		
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = PlayerData->MovementSpeed_Walk;
	}
}

void UPC_ActionComponent::Roll(bool bPressed)
{
	if (bPressed && !IsRolling)
	{
		if (!CanAction(EPC_ActionType::Roll))
			return;
		
		if(!TryConsumeStaminaOnActionStart(EPC_ActionType::Roll))
			return;
		
		const APlayerController* PlayerController = CastChecked<APlayerController>(OwnerCharacter->GetController());
		const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
		UPC_BattleComponent* BattleComponent = Interface->GetBattleComponent();
		check(BattleComponent);

		UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
		check(PlayerData);
		
		AddLock(EPC_LockCauseType::Roll, EPC_ActionType::Move);
		AddLock(EPC_LockCauseType::Roll, EPC_ActionType::Attack);
		AddLock(EPC_LockCauseType::Roll, EPC_ActionType::Jump);
		AddLock(EPC_LockCauseType::Roll, EPC_ActionType::Guard);
		
		IsRolling = true;

		const FRotator Rotation = PlayerController->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		FVector RollDir = FVector::ZeroVector;
		RollDir += ForwardDirection * InputVector.Y;
		RollDir += RightDirection * InputVector.X;

		OwnerCharacter->SetActorRotation(RollDir.Rotation());
		
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		check(AnimInstance);
		
		AnimInstance->StopAllMontages(0.1f);
		OwnerCharacter->PlayAnimMontage(PlayerData->RollMontage);
		FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnMontageEnd);
		AnimInstance->Montage_SetEndDelegate(EndDelegate);
		
		BattleComponent->EndTrace();
	}
}

void UPC_ActionComponent::SwapWeapon(bool bPressed)
{
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	UPC_BattleComponent* BattleComponent = Interface->GetBattleComponent();
	check(BattleComponent);
	BattleComponent->EndTrace();

	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	check(AnimInstance);
	AnimInstance->StopAllMontages(0.1f);
	OwnerCharacter->PlayAnimMontage(PlayerData->WeaponChangeMontage);
	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnMontageEnd);
}

void UPC_ActionComponent::Assassinate(bool IsPressed)
{
	if (!IsPressed)
		return;

	if(IsAssassinating)
		return;
	
	IsAssassinating = true;

	if (!CanAction(EPC_ActionType::Assassinate))
		return;

	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	check(Interface);
	
	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	UPC_BattleComponent* BattleComponent = Interface->GetBattleComponent();
	check(BattleComponent);

	if(!BattleComponent->TryAssassinate())
		return;
	
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	check(AnimInstance);
	AnimInstance->StopAllMontages(0.1f);
	OwnerCharacter->PlayAnimMontage(PlayerData->BackstabMontage);
	
	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &ThisClass::OnMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate);
	
	AddAllLock(EPC_LockCauseType::Assassinate);
}

bool UPC_ActionComponent::CanAction(EPC_ActionType InActionType)
{
	if (IsLocked(InActionType))
		return false;

	return true;
}

bool UPC_ActionComponent::IsLocked(EPC_ActionType InLockType)
{
	for (const auto& Lock :LockData)
	{
		if (Lock.LockType == InLockType)
			return true;
	}

	return false;
}

bool UPC_ActionComponent::IsGuarded()
{
	//Guard 분기
	if(IsInSpecialAction)
	{
		if (CanAction(EPC_ActionType::Guard))
			return true;
		
	}
	return false;
}

void UPC_ActionComponent::AddLock(EPC_LockCauseType InLockCauseType, EPC_ActionType InLockType)
{
	const FPC_LockData Data(InLockCauseType, InLockType);
	LockData.Add(Data);
}

void UPC_ActionComponent::AddAllLock(EPC_LockCauseType InLockCauseType)
{
	for (int i = static_cast<uint8>(EPC_ActionType::Move); i < static_cast<uint8>(InLockCauseType); i++)
	{
		AddLock(InLockCauseType, static_cast<EPC_ActionType>(i));
	}
}

void UPC_ActionComponent::ReleaseLock(EPC_LockCauseType InLockCauseType, EPC_ActionType InLockType)
{
	const int32 Index = LockData.IndexOfByPredicate([InLockCauseType, InLockType](const FPC_LockData& Data)
	{
		return Data.LockType == InLockType && Data.LockCauseType == InLockCauseType;
	});

	//원하는 값을 찾았다.
	if (Index != INDEX_NONE)
	{
		LockData.RemoveAt(Index);
	}
}

void UPC_ActionComponent::ForceReleaseLock(EPC_LockCauseType InLockCauseType)
{
	LockData.RemoveAll([InLockCauseType](const FPC_LockData& Data)
{
	return Data.LockCauseType == InLockCauseType;
});
}

void UPC_ActionComponent::ResetAllLock()
{
	LockData.Empty();
}

void UPC_ActionComponent::ProcessFreeMove()
{
	const APlayerController* PlayerController = CastChecked<APlayerController>(OwnerCharacter->GetController());
	
	const FRotator Rotation = PlayerController->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	OwnerCharacter->AddMovementInput(ForwardDirection, InputVector.Y);
	OwnerCharacter->AddMovementInput(RightDirection, InputVector.X);
}

void UPC_ActionComponent::ProcessLockOnMove()
{
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	const UPC_LockOnComponent* LockOnComponent = Interface->GetLockOnComponent();
	check(LockOnComponent);
	
	if (AActor* TargetActor = LockOnComponent->GetLockTarget())
	{
		const FVector MyLocation = OwnerCharacter->GetActorLocation();
		const FVector TargetLocation = TargetActor->GetActorLocation();

		const FVector ToTarget = (TargetLocation - MyLocation).GetSafeNormal();

		const FVector OrbitRight = FVector::CrossProduct(FVector::UpVector, ToTarget); 
		const FVector OrbitForward = ToTarget;                                        

		FVector MoveDir = OrbitRight * InputVector.X + OrbitForward * InputVector.Y;
		MoveDir.Normalize();

		OwnerCharacter->AddMovementInput(MoveDir);

		FRotator LookAtRot = (TargetLocation - MyLocation).Rotation();
		LookAtRot.Pitch = 0.f; 

		const FRotator CurrentRot = OwnerCharacter->GetActorRotation();
		const FRotator NewRot = FMath::RInterpTo(CurrentRot, LookAtRot, GetWorld()->GetDeltaSeconds(), 10.f);
			 	
		OwnerCharacter->SetActorRotation(NewRot);
	}
}

void UPC_ActionComponent::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	UPC_BattleComponent* BattleComponent = Interface->GetBattleComponent();
	check(BattleComponent);
	
	if (Montage == PlayerData->RollMontage)
	{
		IsRolling = false;
		ForceReleaseLock(EPC_LockCauseType::Roll);
		ForceReleaseLock(EPC_LockCauseType::Attack);
	}

	if(Montage == PlayerData->BackstabMontage)
	{
		IsAssassinating = false;
		ForceReleaseLock(EPC_LockCauseType::Assassinate);
	}

	if(Montage == PlayerData->WeaponChangeMontage)
	{
		if (USkeletalMeshComponent* MeshComp = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr)
		{
			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				AnimInst->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ThisClass::UPC_ActionComponent::OnMontageSwapWeaponBegin);
			}
		}
	}
	
	//if (Montage == CurrentAttackMontage)
	//{
	//	BattleComponent->EndTrace();
	//}
}

void UPC_ActionComponent::ComboAttackSave()
{
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	TArray<UAnimMontage*>& AttackMontages = PlayerData->AttackMontages;
	
	if (SaveAttack)
	{
		if(!TryConsumeStaminaOnActionStart(EPC_ActionType::Attack))
			return;
		
		SaveAttack = false;
		AttackCount++;
		
		if (AttackCount > AttackMontages.Num())
		{
			AttackCount = 0;
		}

		if (AttackMontages.IsValidIndex(AttackCount - 1))
		{
			//CurrentAttackMontage = AttackMontages[AttackCount - 1];
			OwnerCharacter->PlayAnimMontage(AttackMontages[AttackCount - 1]);
		}

		RotateToControlRotation();
	}
	else
	{
		ForceReleaseLock(EPC_LockCauseType::Attack);
	}
}

void UPC_ActionComponent::ResetCombo()
{
	AttackCount = 0;
	SaveAttack = false;
	IsAttacking = false;
	ForceReleaseLock(EPC_LockCauseType::Attack);
}

void UPC_ActionComponent::RotateToControlRotation()
{
	const APlayerController* PlayerController = CastChecked<APlayerController>(OwnerCharacter->GetController());
	FRotator ControlRotation = PlayerController->GetControlRotation();
	ControlRotation.Pitch = 0.f;

	OwnerCharacter->SetActorRotation(ControlRotation);
}

bool UPC_ActionComponent::IsLastAttack() const
{
	return AttackCount == AttackMaxCount;
}

const FPC_ActionStaminaData* UPC_ActionComponent::GetActionStaminaData(EPC_ActionType Type) const
{
	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	UPC_PlayerDataAsset* PlayerData = Interface->GetPlayerData();
	check(PlayerData);

	//내가 가지고 있는거
	for(const FPC_ActionStaminaData& Data : PlayerData->ActionStaminaDatas)
	{
		if(Data.ActionType == Type)
		{
			return &Data;
		}
	}

	return nullptr;
}

bool UPC_ActionComponent::TryConsumeStaminaOnActionStart(EPC_ActionType InActionType)
{
	const FPC_ActionStaminaData* ActionStamina = GetActionStaminaData(InActionType);
	if(ActionStamina == nullptr)
		return true;

	const float StartCost = ActionStamina->StartCost;
	if(StartCost <= 0.f)
		return true;
	
	const IPC_CharacterInterface* CharacterInterface = CastChecked<IPC_CharacterInterface>(GetOwner());
	UPC_StatComponent* StatComponent = CharacterInterface->GetStatComponent();
	check(StatComponent);

	return StatComponent->TryConsumeStamina(StartCost);
}

void UPC_ActionComponent::DrawFeetSpheres(ACharacter* Char, float Radius, float Life)
{
	if (!Char) return;

	const USkeletalMeshComponent* Mesh = Char->GetMesh();
	if (!Mesh) return;

	static const FName SockL(TEXT("foot_l"));
	static const FName SockR(TEXT("foot_r"));

	const FVector L = Mesh->DoesSocketExist(SockL) ? Mesh->GetSocketLocation(SockL) : Char->GetActorLocation();
	const FVector R = Mesh->DoesSocketExist(SockR) ? Mesh->GetSocketLocation(SockR) : Char->GetActorLocation();

	UWorld* World = Char->GetWorld();
	if (!World) return;

	// 분할 수(segments) 12~16 정도면 충분
	DrawDebugSphere(World, L, Radius, 16, FColor::Green, false, Life, 0, 1.f);
	DrawDebugSphere(World, R, Radius, 16, FColor::Green, false, Life, 0, 1.f);
}

void UPC_ActionComponent::OnMontageSwapWeaponBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (NotifyName != FName("SwapWeaponPoint"))
		return;

	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	if(Interface)
	{
		UPC_BattleComponent* BattleComponent = Interface->GetBattleComponent();
		check(BattleComponent);
		BattleComponent->SwapWeapon();
	}
}

