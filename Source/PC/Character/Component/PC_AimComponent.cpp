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

		const FVector TargetOffset = FPC_GameUtil::GetCameraData(CurrentCameraType)->SocketOffset;
		const FRotator TargetArmRotation = FPC_GameUtil::GetCameraData(CurrentCameraType)->CameraRot;
		const float TargetArmLength = FPC_GameUtil::GetCameraData(CurrentCameraType)->TargetArmLength;
		const float TargetFOV = FPC_GameUtil::GetCameraData(CurrentCameraType)->CameraFov;

		// 보간 처리
		const FVector NewOffset = FMath::VInterpTo(SpringArmComponent->SocketOffset, TargetOffset, DeltaTime, 30.f);
		const FRotator NewRot = FMath::RInterpTo(SpringArmComponent->GetRelativeRotation(), TargetArmRotation, DeltaTime, 30.f);
		const float NewLen = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetArmLength, DeltaTime, 30.f);
		const float NewFOV = FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, 30.f);
		
		SpringArmComponent->SocketOffset = NewOffset;
		CameraComponent->SetRelativeRotation(NewRot);
		CameraComponent->FieldOfView = NewFOV;
		SpringArmComponent->TargetArmLength = NewLen;

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
