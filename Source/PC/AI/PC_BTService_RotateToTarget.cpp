// Fill out your copyright notice in the Description page of Project Settings.
#include "PC_BTService_RotateToTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

//ex) 게걸음 할때 플레이어 바라보도록 할때 
UPC_BTService_RotateToTarget::UPC_BTService_RotateToTarget()
{
	NodeName = TEXT("SetFocus Node");
}

void UPC_BTService_RotateToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACharacter* ControllingCharacter = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if(!ControllingCharacter)
		return;

	const APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if(!Target)
		return;

	//GetSafeNormal2D는 하늘을 바라보지 않게
	FVector LookAtRot = (Target->GetActorLocation() - ControllingCharacter->GetActorLocation()).GetSafeNormal2D(); 
	
	//보간
	const FRotator CurrentRot = ControllingCharacter->GetActorRotation();
	const FRotator NewRot = FMath::RInterpTo(CurrentRot, LookAtRot.Rotation(), DeltaSeconds, 1);

	ControllingCharacter->SetActorRotation(NewRot);
}
