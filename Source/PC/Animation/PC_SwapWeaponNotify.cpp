// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_SwapWeaponNotify.h"

#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

void UPC_SwapWeaponNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(MeshComp->GetOwner()))
		{
			UPC_BattleComponent* BattleComponent = CharacterInterface->GetBattleComponent();
			if(BattleComponent)
			{
				BattleComponent->SwapWeapon();
			}
		}
	}
}
