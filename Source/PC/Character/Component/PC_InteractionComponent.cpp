#include "PC_InteractionComponent.h"
#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_InteractionComponent::UPC_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	//OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UPC_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UPC_InteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(IPC_PlayerCharacterInterface* PlayerCharacterInterface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
	{
		UPC_ActionComponent* ActionComponent = PlayerCharacterInterface->GetActionComponent();
		check(ActionComponent);

		//UPC_InteractionComponent* InteractionComponent = PlayerCharacterInterface->GetInteractionComponent();
		//if(!InteractionComponent)
		//	return;

		APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
		if(!PlayerController)
			return;

		if(OverlappingActors.Num() > 0)
		{
			AActor* TargetActor = FPC_GameUtil::GetBestTargetByViewAngle(PlayerController, OverlappingActors, true,90);
			if(AssassinateTarget != TargetActor)
			{
				for(AActor* Actor : OverlappingActors)
				{
					IPC_CharacterWidgetInterface* CharacterWidgetInterface = Cast<IPC_CharacterWidgetInterface>(Actor);
					check(CharacterWidgetInterface);

					CharacterWidgetInterface->OnSelectedAssassinateTarget(Actor == TargetActor); 
				}
			}

			AssassinateTarget = TargetActor;
		}
	}
}

void UPC_InteractionComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OverlappingActors.Add(OtherActor);
}

void UPC_InteractionComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OverlappingActors.Remove(OtherActor);

	if(IPC_CharacterWidgetInterface* CharacterWidgetInterface = Cast<IPC_CharacterWidgetInterface>(OtherActor))
	{
		CharacterWidgetInterface->OnSelectedAssassinateTarget(false);
	}

	if(OtherActor == AssassinateTarget)
	{
		AssassinateTarget = nullptr;
	}
}
