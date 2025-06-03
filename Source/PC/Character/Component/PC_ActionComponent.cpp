// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_ActionComponent.h"

#include "PC_BattleComponent.h"
#include "PC_LockOnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"

void UPC_ActionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
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

	TArray<UAnimMontage*>& AttackMontages = PlayerData->AttackMontages;

	if (IsAttacking)
	{
		SaveAttack = true;
	}
	else
	{
		IsAttacking = true;
		AttackCount++;
		
		if (AttackCount > AttackMontages.Num())
		{
			AttackCount = 0;
		}

		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		check(AnimInstance);

		if (AttackMontages.IsValidIndex(AttackCount - 1))
		{
			OwnerCharacter->PlayAnimMontage(AttackMontages[AttackCount - 1]);
		}

		RotateToControlRotation();

		AddLock(EPC_LockCauseType::Attack, EPC_ActionType::Move);
		AddLock(EPC_LockCauseType::Attack, EPC_ActionType::Jump);
	}
}

void UPC_ActionComponent::Guard(bool bPressed)
{
	if (bPressed && !IsGuarding)
	{
		if (!CanAction(EPC_ActionType::Guard))
			return;
		
		IsGuarding = true;
	}
	else if (!bPressed && IsGuarding)
	{
		IsGuarding = false;
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

		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = PlayerData->MovementSpeed_Run;
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

void UPC_ActionComponent::AddLock(EPC_LockCauseType InLockCauseType, EPC_ActionType InLockType)
{
	const FPC_LockData Data(InLockCauseType, InLockType);
	LockData.Add(Data);
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
