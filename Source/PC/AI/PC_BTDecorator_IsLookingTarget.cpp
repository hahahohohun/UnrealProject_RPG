// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTDecorator_IsLookingTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"


UPC_BTDecorator_IsLookingTarget::UPC_BTDecorator_IsLookingTarget()
{
	NodeName = TEXT("IsLookingTarget");
}

bool UPC_BTDecorator_IsLookingTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	APawn* ControllingPanw = OwnerComp.GetAIOwner()->GetPawn();
	if(!ControllingPanw)
		return false;

	const APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if(!Target)
		return false;

	const FVector Forward = ControllingPanw->GetActorForwardVector();
	const FVector ToTarget = (Target->GetActorLocation() - ControllingPanw->GetActorLocation()).GetSafeNormal();
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(Forward.Dot(ToTarget)));
	
	bResult = Angle < 45.f;
	return bResult;
}

