// Fill out your copyright notice in the Description page of Project Settings.
#include "PC_PickableItem.h"

#include "Components/SphereComponent.h"
#include "PC/Character/Component/PC_StatusEffectComponent.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

// Sets default values
APC_PickableItem::APC_PickableItem()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	TriggerCollision->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APC_PickableItem::BeginPlay()
{
	Super::BeginPlay();

	if (StatusEffectId > -1)
	{
		if (!TriggerCollision->OnComponentBeginOverlap.IsAlreadyBound(this, &ThisClass::OnBeginOverlap))
			TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	}
}

void APC_PickableItem::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                      const FHitResult& SweepResult)
{
	FPC_StatusEffectTableRow* StatusEffectTableRow = FPC_GameUtil::GetStatusEffectData(StatusEffectId);
	check(StatusEffectTableRow);

	if (IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OtherActor))
	{
		CharacterInterface->OnApplyStatusEffect(StatusEffectId);
		
		if(PickupSound)
			FPC_GameUtil::PlaySFXAtLocation(GetWorld(), PickupSound, GetActorLocation());
		
		Destroy();
	}
	else
	{
		return;
	}
}
