// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_TrailEffect.h"

#include "GameFramework/Character.h"
#include "Particles/ParticleSystemComponent.h"
#include "PC/Interface/PC_CharacterInterface.h"

void UPC_TrailEffect::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (MeshComp)
	{
		if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(MeshComp->GetOwner()))
		{
			if (bWeaponTrail && CharacterInterface->HasWeapon())
			{
				UStaticMeshComponent* StaticMeshComponent = bRight ? 
					CharacterInterface->GetWeapon_R_StaticMeshComponent() : CharacterInterface->GetWeapon_L_StaticMeshComponent();
				
				check(StaticMeshComponent);

				TPair<FName, FName> WeaponTraceNames = CharacterInterface->GetWeaponTraceNames(bRight);

				FVector StartPos = StaticMeshComponent->GetSocketLocation(WeaponTraceNames.Key);
				FVector EndPos = StaticMeshComponent->GetSocketLocation(WeaponTraceNames.Value);

				if (UFXSystemComponent* FXSystemComponent = GetSpawnedEffect(MeshComp))
				{
					FXSystemComponent->SetVectorParameter(TEXT("StartTrail"), StartPos);
					FXSystemComponent->SetVectorParameter(TEXT("EndTrail"), EndPos);
				}
			}
			else if (!bWeaponTrail)
			{
				ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
				check(OwnerCharacter);

				USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
				check(SkeletalMeshComponent);

				FVector StartPos = SkeletalMeshComponent->GetSocketLocation(BodyTrailBoneName_Start);
				FVector EndPos = SkeletalMeshComponent->GetSocketLocation(BodyTrailBoneName_End);

				if (UFXSystemComponent* FXSystemComponent = GetSpawnedEffect(MeshComp))
				{
					FXSystemComponent->SetVectorParameter(TEXT("StartTrail"), StartPos);
					FXSystemComponent->SetVectorParameter(TEXT("EndTrail"), EndPos);
				}
			}
		}
	}
}

UFXSystemComponent* UPC_TrailEffect::SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const
{
	UFXSystemComponent* System = Super::SpawnEffect(MeshComp, Animation);
	return System;
}
