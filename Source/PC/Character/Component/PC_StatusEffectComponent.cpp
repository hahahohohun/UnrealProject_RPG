#include "PC_StatusEffectComponent.h"

UPC_StatusEffectComponent::UPC_StatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UPC_StatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPC_StatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPC_StatusEffectComponent::RemoveEffect(uint32 StatusEffectID)
{
	
}

void UPC_StatusEffectComponent::ApplyStatusEffect(uint32 DataId)
{
	
}
