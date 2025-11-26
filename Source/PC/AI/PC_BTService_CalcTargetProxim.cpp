// Fill out your copyright notice in the Description page of Project Settings.
#include "PC_BTService_CalcTargetProxim.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Utills/PC_GameUtill.h"

//ex) 게걸음 할때 플레이어 바라보도록 할때 
UPC_BTService_CalcTargetProxim::UPC_BTService_CalcTargetProxim()
{
	NodeName = TEXT("Calc Target Proximity Node");
	bNotifyTick = true;
	Interval = 0.1f;
}

void UPC_BTService_CalcTargetProxim::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!ControllingPawn)
		return;

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if (!AIPawn)
		return;

	APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (!Target)
		return;

	EPC_ProximityType TargetProximity = FPC_GameUtil::GetTargetProximity(
		ControllingPawn, Target, UnderRange, NearRange, MiddleRange, FVector::ZeroVector);

	OwnerComp.GetBlackboardComponent()->
	          SetValueAsEnum(TEXT("TargetProximityType"), static_cast<uint8>(TargetProximity));
}
