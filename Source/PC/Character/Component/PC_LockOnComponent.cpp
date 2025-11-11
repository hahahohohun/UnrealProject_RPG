#include "PC_LockOnComponent.h"

#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_LockOnComponent::UPC_LockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	//락온 할 시야각
	TargetDetectRadius = 1200.f;
	TargetDetectAngle = 90.f;
}

void UPC_LockOnComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsLockOnMode())
	{
		ACharacter* Owner = Cast<ACharacter>(GetOwner());
		check(Owner);

		APlayerController* PlayerController = Cast<APlayerController>(Owner->GetController());
		check(PlayerController);

		if (LockedTarget.Get())
		{
			const FVector LockTargetLocation = LockedTarget->GetActorLocation();
			const FVector OwnerLocation = GetOwner()->GetActorLocation();

			const FRotator CurrentRot = PlayerController->GetControlRotation();
			FRotator TargetRot = (LockTargetLocation - OwnerLocation).Rotation();

			TargetRot.Pitch = FMath::Clamp(TargetRot.Pitch, -15.f, 5.f); // 혹은 고정값 보간
			FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 10.f);

			PlayerController->SetControlRotation(NewRot);
		}
	}
}

void UPC_LockOnComponent::LockOn()
{
	SetLockOnMode(!IsLockOnMode());
}

APawn* UPC_LockOnComponent::FindTarget()
{
	UWorld* CurrentWorld = GetWorld();
	check(CurrentWorld);

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	check(Owner);
	
	APlayerController* PlayerController = Cast<APlayerController>(Owner->GetController());
	check(PlayerController);

	//플레이어가 바라보는 
	FVector CameraForward = PlayerController->GetControlRotation().Vector();
	CameraForward.Z = 0.f;

	FVector OwnerLocation = Owner->GetActorLocation();
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	//반경 TargetDetectRadius 으로 Detection
	TArray<FOverlapResult> OverlapResult;
	CurrentWorld->OverlapMultiByChannel(OverlapResult, OwnerLocation, FQuat::Identity,
		ECC_GameTraceChannel3, FCollisionShape::MakeSphere(TargetDetectRadius),QueryParams);

	TArray<AActor*> Actors;
	for(FOverlapResult& Result : OverlapResult)
	{
		Actors.Add(Result.GetActor());
	}

	return Cast<APawn>(FPC_GameUtil::GetBestTargetByViewAngle(PlayerController,Actors,false, TargetDetectRadius));
}

void UPC_LockOnComponent::LockTarget(APawn* InActor)
{
	if(!InActor)
		return;

	LockedTarget = InActor;
	
	if(IPC_CharacterWidgetInterface* Character = Cast<IPC_CharacterWidgetInterface>(LockedTarget))
	{
		Character->OnLocked(true);
	}
}

void UPC_LockOnComponent::ClearTarget()
{
	if (IPC_CharacterWidgetInterface* Character = Cast<IPC_CharacterWidgetInterface>(LockedTarget))
	{
		Character->OnLocked(false);
	}
	
	LockedTarget = nullptr;
}

void UPC_LockOnComponent::SetLockOnMode(bool bEnable)
{
	bLockOnMode = bEnable;
	
	if (bEnable)
	{
		if (APawn* Target = FindTarget())
		{
			LockTarget(Target);
		}
		else
		{
			bLockOnMode = false;
		}
	}

	if (!bLockOnMode)
	{
		ClearTarget();
	}
}
