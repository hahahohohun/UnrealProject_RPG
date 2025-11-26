#include "PC_LockOnComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
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
			const FVector OwnerLocation = Owner->GetActorLocation();
			const FVector LookAtPoint = GetLockOnViewPoint(LockedTarget.Get());

			const FRotator CurrentRot = PlayerController->GetControlRotation();
			FRotator TargetRot = (LookAtPoint - OwnerLocation).Rotation();

			// 큰 보스일수록 pitch 제한 완화
			TargetRot.Pitch = FMath::Clamp(TargetRot.Pitch, -25.f, 10.f);

			FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 10.f);

			PlayerController->SetControlRotation(NewRot);
		}
	}

	// 락온 중이면 보스 크기에 맞춰 암 길이 조정,
	// 락온이 아니면 기본 길이로 복원
	UpdateCameraArmLength(DeltaTime);
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
		TargetOverViewPoint = GetLockOnViewPoint(LockedTarget.Get());
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
	TargetOverViewPoint = FVector::ZeroVector;
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

FVector UPC_LockOnComponent::GetLockOnViewPoint(AActor* TargetActor)
{
	if (!TargetActor)
		return FVector::ZeroVector;

	USkeletalMeshComponent* Mesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh)
		return TargetActor->GetActorLocation();

	const float Height = Mesh->Bounds.BoxExtent.Z; // 절반 높이
	const float OffsetZ = Height * 0.6f;           // 머리 근처로 조절

	return TargetActor->GetActorLocation() + FVector(0, 0, OffsetZ);
}

float UPC_LockOnComponent::GetTargetHeight(AActor* TargetActor) const
{
	if (!TargetActor)
		return 0.f;

	if (USkeletalMeshComponent* Mesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
	{
		// Bounds.BoxExtent.Z 는 절반 높이이므로 * 2
		return Mesh->Bounds.BoxExtent.Z * 2.f;
	}

	return 0.f;
}

float UPC_LockOnComponent::GetDesiredArmLength(AActor* TargetActor) const
{
	if (!TargetActor || !LockedTarget.IsValid())
	{
		// 타겟 없으면 기본 암 길이로
		return DefaultArmLength;
	}

	const float TargetHeight = GetTargetHeight(TargetActor);
	if (TargetHeight <= 0.f)
	{
		return DefaultArmLength;
	}

	// 예시 매핑:
	// 사람형(약 180) 기준 → 기본값 근처
	// 그보다 큰 보스일수록 점점 MaxArmLength에 가까워지게
	const float BaseHeight = 180.f;   // 사람 키 기준
	const float MaxHeightForScale = 500.f; // 이 이상은 그냥 최대 거리

	const float Normalized =
		FMath::Clamp((TargetHeight - BaseHeight) / (MaxHeightForScale - BaseHeight), 0.f, 1.f);

	const float DesiredArmLength =
		FMath::Lerp(DefaultArmLength, MaxArmLength, Normalized);

	// 최소/최대 클램프
	return FMath::Clamp(DesiredArmLength, MinArmLength, MaxArmLength);
}

void UPC_LockOnComponent::UpdateCameraArmLength(float DeltaTime)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
		return;

	USpringArmComponent* SpringArm = Owner->FindComponentByClass<USpringArmComponent>();
	if (!SpringArm)
		return;

	// 처음 한 번 기본 길이 캐싱
	if (!bCachedDefaultArmLength)
	{
		DefaultArmLength = SpringArm->TargetArmLength;
		bCachedDefaultArmLength = true;
	}

	float TargetArmLength = DefaultArmLength;

	if (IsLockOnMode() && LockedTarget.IsValid())
	{
		TargetArmLength = GetDesiredArmLength(LockedTarget.Get());
	}

	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		TargetArmLength,
		DeltaTime,
		ArmInterpSpeed
	);
}
