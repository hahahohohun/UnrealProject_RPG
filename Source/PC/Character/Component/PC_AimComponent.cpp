#include "PC_AimComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
//#include "PC/Data/.h"
#include "PC/Data/PC_CameraDataAsset.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_AimComponent::UPC_AimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPC_AimComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentCameraType = EPC_CameraType::Normal;
	OwnerCharacter = CastChecked<ACharacter>(GetOwner());

	const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
	USpringArmComponent* SpringArmComponent = Interface->GetSpringArmComponent();
	check(SpringArmComponent);

	UCameraComponent* CameraComponent = Interface->GetCameraComponent();
	check(CameraComponent);

	const FVector TargetOffset = FPC_GameUtil::GetCameraData(CurrentCameraType)->SocketOffset;
	const FRotator TargetArmRotation = FPC_GameUtil::GetCameraData(CurrentCameraType)->CameraRot;
	const float TargetArmLength = FPC_GameUtil::GetCameraData(CurrentCameraType)->TargetArmLength;
	const float TargetFOV = FPC_GameUtil::GetCameraData(CurrentCameraType)->CameraFov;
	
	SpringArmComponent->SocketOffset = TargetOffset;
	CameraComponent->SetRelativeRotation(TargetArmRotation);
	CameraComponent->FieldOfView = TargetFOV;
	SpringArmComponent->TargetArmLength = TargetArmLength;
}

void UPC_AimComponent::OnCameraAnimFinished()
{
	bCameraAnimPlaying = false;
	SwitchCamera(PrevCameraType);
}

void UPC_AimComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bCameraBlending)
	{
		const IPC_PlayerCharacterInterface* Interface = CastChecked<IPC_PlayerCharacterInterface>(GetOwner());
		USpringArmComponent* SpringArmComponent = Interface->GetSpringArmComponent();
		check(SpringArmComponent);

		UCameraComponent* CameraComponent = Interface->GetCameraComponent();
		check(CameraComponent);

		const UPC_CameraDataAsset* CameraDataAsset = FPC_GameUtil::GetCameraData(CurrentCameraType);
		const FVector TargetOffset = CameraDataAsset->SocketOffset;
		const FRotator TargetArmRotation = CameraDataAsset->CameraRot;
		const float TargetArmLength = CameraDataAsset->TargetArmLength;
		const float TargetFOV = CameraDataAsset->CameraFov;

		// 보간 처리
		const float Speed = CameraDataAsset->BlendInterp;
		const FVector NewOffset = FMath::VInterpTo(SpringArmComponent->SocketOffset, TargetOffset, DeltaTime, Speed);
		const FRotator NewRot = FMath::RInterpTo(CameraComponent->GetRelativeRotation(), TargetArmRotation, DeltaTime, Speed);
		const float NewLen = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetArmLength, DeltaTime, Speed);
		const float NewFOV = FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, Speed);
		
		SpringArmComponent->SocketOffset = NewOffset;
		CameraComponent->SetRelativeRotation(NewRot);
		CameraComponent->FieldOfView = NewFOV;
		SpringArmComponent->TargetArmLength = NewLen;
		//CameraComponent->SetRelativeTransform(NewRot);

		if ((TargetOffset - NewOffset).Length() <= 0.5f)
		{
			bCameraBlending = false;	
		}
	}
	
	if (CurrentCameraType == EPC_CameraType::Aim)
	{
		CalcAimOffset(DeltaTime);
	}
}

void UPC_AimComponent::SwitchCamera(EPC_CameraType CameraType)
{
	CurrentCameraType = CameraType;
	bCameraBlending = true;
}

void UPC_AimComponent::PlayCameraAnim(EPC_CameraType CameraType, float Time)
{
	if (CurrentCameraType == CameraType)
		return;

	// Aim 중에는 애니메이션 X
	if (CurrentCameraType == EPC_CameraType::Aim)
		return;

	if (bCameraAnimPlaying)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CameraAnimTimerHandle);
		}
	}

	PrevCameraType = CurrentCameraType;   // 나중에 되돌릴 타입 저장
	bCameraAnimPlaying = true;

	// 줌인용 타입으로 전환 (여기서 bCameraBlending = true; 가 이미 처리됨)
	SwitchCamera(CameraType);

	// Time 후에 원복
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CameraAnimTimerHandle,
			this,
			&UPC_AimComponent::OnCameraAnimFinished,
			Time,
			false
		);
	}
}

void UPC_AimComponent::CalcAimOffset(float DeltaTime)
{
	check(OwnerCharacter.Get());

	FRotator ControlRotation = OwnerCharacter->GetControlRotation();
	FRotator ActorRotation = OwnerCharacter->GetActorRotation();

	FRotator NormalizedDeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

	float NewPitch = FMath::FInterpTo(AimOffsetRotation.Pitch, NormalizedDeltaRotation.Pitch, DeltaTime, 30.f);
	float NewYaw = FMath::FInterpTo(AimOffsetRotation.Yaw, NormalizedDeltaRotation.Yaw, DeltaTime, 30.f);

	AimOffsetRotation = FRotator(NewPitch, NewYaw, 0.f);
}
