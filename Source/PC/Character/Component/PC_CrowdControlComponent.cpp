#include "PC_CrowdControlComponent.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PC/Data/PC_CharacterDataAsset.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_CrowdControlComponent::UPC_CrowdControlComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPC_CrowdControlComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UPC_CrowdControlComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Tick_PlayCrowdControl(DeltaTime);
}

void UPC_CrowdControlComponent::Tick_PlayCrowdControl(float DeltaTime)
{
	ProcessCC();

	CrowdControlInfo.ElapsedTime += DeltaTime;
	//
	if(CrowdControlInfo.bValid && CrowdControlInfo.ElapsedTime > CrowdControlInfo.LifeTime)
	{
		StopCC();
	}
}

void UPC_CrowdControlComponent::ProcessCC()
{
}

void UPC_CrowdControlComponent::RequestPlayerCC(uint32 CrowdControlId, AActor* Causer)
{
	FPC_CrowdControlTableRow* CrowdControlTableRow = FPC_GameUtil::GetCrowdControlData(CrowdControlId);
	check(CrowdControlTableRow);

	FPC_CrowdControlInfo Info;
	Info.LifeTime = CrowdControlTableRow->Duration;
	Info.ElapsedTime = 0.f;
	Info.CrowdControlType = CrowdControlTableRow->CrowdControlType;
	Info.CrowdControlDataId = CrowdControlTableRow->DataId;
	Info.Causer = Causer;

	Info.CauserPos = Causer->GetActorLocation();
	Info.CauserRot = Causer->GetActorRotation();
	Info.StartPos = OwnerCharacter->GetActorLocation();
	Info.StartRot = OwnerCharacter->GetActorRotation();

	if(!CanPlayCC(Info))
	{
		return;
	}

	//이미 실행중인 cc가 있을경우 id가 다를떄에만 stop
	if(CrowdControlInfo.bValid && CrowdControlInfo.CrowdControlDataId != CrowdControlId)
	{
		StopCC();
	}

	PlayCC(Info);
}

bool UPC_CrowdControlComponent::CanPlayCC(FPC_CrowdControlInfo& info)
{
	return true;
}

void UPC_CrowdControlComponent::PlayCC(FPC_CrowdControlInfo& info)
{
	//데이터가 다를 경우 fx 스타트
	const bool ShouldPlayFx = CrowdControlInfo.CrowdControlDataId != info.CrowdControlDataId
	|| !CrowdControlInfo.bValid;

	//캐싱
	CrowdControlInfo = info;
	OnStartCC();

	if(ShouldPlayFx)
		PlayFX(info);
	else
		info.SpawnedFx = CrowdControlInfo.SpawnedFx;

	CrowdControlInfo = info;
	OnStartCC();
	
	CrowdControlInfo.bValid = true;
	OnStartCCDelegate.Broadcast(CrowdControlInfo.CrowdControlType, CrowdControlInfo.Causer.Get());
}

void UPC_CrowdControlComponent::StopCC()
{
	OnStopCC();

	CrowdControlInfo.bValid = false;
	OnEndCCDelegate.Broadcast(CrowdControlInfo.CrowdControlType, CrowdControlInfo.Causer.Get());
}

void UPC_CrowdControlComponent::OnStartCC()
{
	FPC_CrowdControlTableRow* CrowdControlTableRow = FPC_GameUtil::GetCrowdControlData(CrowdControlInfo.CrowdControlDataId);
	check(CrowdControlTableRow);

	IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwnerCharacter);
	check(CharacterInterface);

	if(AAIController* AaiController = Cast<AAIController>(OwnerCharacter->GetController()))
	{
		AaiController->StopMovement();
	}
	
	if(CrowdControlInfo.CrowdControlType == EPC_CrowdControlType::Freeze)
	{
		USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
		check(SkeletalMeshComponent);
	
		OwnerCharacter->GetCharacterMovement()->DisableMovement();
		SkeletalMeshComponent->SetComponentTickEnabled(false);
	}

	if(UPC_CharacterDataAsset* CharacterDataAsset = CharacterInterface->GetCharacterDataAsset())
	{
		//if(TObjectPtr<UAnimMontage> AnimMontage = CharacterDataAsset->KnockbackAnim)
		//{
		//	USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
		//	check(SkeletalMeshComponent);
	    //
		//	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
		//	check(AnimInstance);
	    //
		//	AnimInstance->StopAllMontages(0.2f);
		//	AnimInstance->Montage_Play(AnimMontage);
		//}
	}

	if(CrowdControlInfo.CrowdControlType == EPC_CrowdControlType::Pushback)
	{
		const FVector StartPos = CrowdControlInfo.StartPos;
		const FVector CauserPos = CrowdControlInfo.CauserPos;

		const FVector ForceDir = (StartPos - CauserPos).GetSafeNormal();
		const float Power = CrowdControlTableRow->Property_0;

		//스킬 사용자를 바라보며 날라가게
		OwnerCharacter->SetActorRotation((-ForceDir.GetSafeNormal2D()).Rotation());
		OwnerCharacter->LaunchCharacter(ForceDir * Power, true, true);
	}
}

void UPC_CrowdControlComponent::OnStopCC()
{
	FPC_CrowdControlTableRow* CrowdControlTableRow = FPC_GameUtil::GetCrowdControlData(CrowdControlInfo.CrowdControlDataId);
	check(CrowdControlTableRow);

	if(CrowdControlInfo.SpawnedFx && CrowdControlInfo.SpawnedFx->IsActive())
	{
		CrowdControlInfo.SpawnedFx->Deactivate();
	}

	if(CrowdControlTableRow->MaterialInstance)
	{
		USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
		check(SkeletalMeshComponent);

		SkeletalMeshComponent->SetOverlayMaterial(nullptr);

		if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwnerCharacter))
		{
			UStaticMeshComponent* Weapon_L = CharacterInterface->GetWeapon_L_StaticMeshComponent();
			check(Weapon_L);
			Weapon_L->SetOverlayMaterial(nullptr);
			
			UStaticMeshComponent* Weapon_R = CharacterInterface->GetWeapon_R_StaticMeshComponent();
			check(Weapon_R);
			Weapon_R->SetOverlayMaterial(nullptr);
		}
	}

	if(CrowdControlInfo.CrowdControlType == EPC_CrowdControlType::Freeze)
	{
		USkeletalMeshComponent* skeletalMeshComponent = OwnerCharacter->GetMesh();
		check(skeletalMeshComponent);

		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		skeletalMeshComponent->SetComponentTickEnabled(true);
	}
}

void UPC_CrowdControlComponent::PlayFX(FPC_CrowdControlInfo& Info)
{
	FPC_CrowdControlTableRow* CrowdControlTableRow = FPC_GameUtil::GetCrowdControlData(Info.CrowdControlDataId);
	check(CrowdControlTableRow);

	if(UNiagaraSystem* NiagaraSystem = CrowdControlTableRow->CrowdControlFX)
	{
		FVector RelativePos = FVector(0.f,0.f,OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		Info.SpawnedFx = FPC_GameUtil::SpawnEffectAttached(NiagaraSystem, OwnerCharacter->GetCapsuleComponent(), NAME_None,
			RelativePos, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	}

	if(CrowdControlTableRow->MaterialInstance)
	{
		USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
		check(SkeletalMeshComponent);

		SkeletalMeshComponent->SetOverlayMaterial(CrowdControlTableRow->MaterialInstance);
		//TODO Weapon Mesh에도 메테리얼 변경 필요
		if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwnerCharacter))
		{
			UStaticMeshComponent* Weapon_L = CharacterInterface->GetWeapon_L_StaticMeshComponent();
			check(Weapon_L);
			Weapon_L->SetOverlayMaterial(CrowdControlTableRow->MaterialInstance);
			
			UStaticMeshComponent* Weapon_R = CharacterInterface->GetWeapon_R_StaticMeshComponent();
			check(Weapon_R);

			Weapon_R->SetOverlayMaterial(CrowdControlTableRow->MaterialInstance);
		}
	}
}

void UPC_CrowdControlComponent::StopFX()
{
}

bool UPC_CrowdControlComponent::IsCrowdControlled()
{
	return  false;

}
