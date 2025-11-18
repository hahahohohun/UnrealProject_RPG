// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_WeaponSparkEffectNotify.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_WeaponSparkEffectNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;
	
	if (MeshComp)
	{
		if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(MeshComp->GetOwner()))
		{
			if (CharacterInterface->HasWeapon())
			{
				CharacterInterface->WeaponSparkEffect(bStart, bRight);
			}	
		}
	}
}
