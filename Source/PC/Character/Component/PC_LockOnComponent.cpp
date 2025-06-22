#include "PC_LockOnComponent.h"

#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

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

	if(IsLockOnMode())
	{
		ACharacter* Owner = Cast<ACharacter>(GetOwner());
		check(Owner);

		APlayerController* PlayerController = Cast<APlayerController>(Owner->GetController());
		check(PlayerController);

		if(LockedTarget.Get())
		{
			const FVector LockTargetLocation = Owner->GetActorLocation();
			const FVector OwnerLocation = GetOwner()->GetActorLocation();

			const FRotator CurrentRotation = PlayerController->GetControlRotation();
			const FRotator TargetRot = (LockTargetLocation - OwnerLocation).Rotation();

			const FRotator NewRot = FMath::RInterpTo(CurrentRotation, TargetRot, DeltaTime, 10.f);

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

	APawn* FoundTarget = nullptr;
	float BestAngle = INT_MAX;

	//사잇값이 가장 좁은 타겟 찾기
	for (FOverlapResult& Result : OverlapResult)
	{
		APawn* ResultPawn = Cast<APawn>(Result.GetActor());
		if (!ResultPawn)
			continue;
		
		FVector ToTargetDir = (Result.GetActor()->GetActorLocation() - OwnerLocation).GetSafeNormal();
		float OffsetAngle = FMath::RadiansToDegrees(FMath::Acos(ToTargetDir.Dot(CameraForward)));
		if (OffsetAngle < TargetDetectAngle)
		{
			if (OffsetAngle < BestAngle)
			{
				FoundTarget = ResultPawn;
				BestAngle = OffsetAngle;
			}
		}
	}
	
	return FoundTarget;
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
