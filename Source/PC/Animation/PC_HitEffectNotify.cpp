// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_HitEffectNotify.h"

#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_HitEffectNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	if (UNiagaraSystem* NiagaraSystem = HitFX)
	{
		auto owner = MeshComp->GetOwner();
		if (!owner)
			return;

		APawn* ControllingPawn = Cast<APawn>(MeshComp->GetOwner());
		if(!ControllingPawn) return;

		
		FVector RelativePos = FVector(0.f, 0.f, ControllingPawn->GetActorScale3D().Z);
		FPC_GameUtil::SpawnEffectAttached(NiagaraSystem, MeshComp,
														   EffectBoneName,
														   RelativePos, FRotator::ZeroRotator,
														   EAttachLocation::SnapToTarget, true);
	
	}
}
