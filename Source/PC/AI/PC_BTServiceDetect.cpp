// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_BTServiceDetect.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTServiceDetect::UPC_BTServiceDetect()
{
	NodeName = TEXT("Detect Task Node");
	Interval = 1.0f; //매초 주기로
}

void UPC_BTServiceDetect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if(!ControllingPawn)
		return;
	
	UWorld* World = ControllingPawn->GetWorld();
	if(!World)
		return;

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if(!AIPawn)
		return;
	
	float DetectRadius = AIPawn->GetAIDetectRange();
	FVector PawnLocation = ControllingPawn->GetActorLocation();

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParam(SCENE_QUERY_STAT(Detect), false, ControllingPawn);

	bool bResult = World->OverlapMultiByChannel(
		OverlapResults,
		PawnLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(DetectRadius),
		CollisionQueryParam);

	if (bResult)
	{
		for (auto const& OverlapResult : OverlapResults)
		{
			APawn* Pawn = Cast<APawn>(OverlapResult.GetActor());
			if (Pawn && Pawn->GetController()->IsPlayerController())
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Pawn);
				
				DrawDebugSphere(World, PawnLocation, DetectRadius, 16, FColor::Green, false, 0.2f);

				DrawDebugPoint(World, Pawn->GetActorLocation(), 10.0f, FColor::Green, false, 0.2f);
				DrawDebugLine(World, ControllingPawn->GetActorLocation(), Pawn->GetActorLocation(), FColor::Green, false, 0.27f);
				return;
			}
		}
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), nullptr);
	DrawDebugSphere(World, PawnLocation, DetectRadius, 16, FColor::Red, false, 0.2f);
}
