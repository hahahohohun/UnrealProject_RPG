#include "PC_BTTask_MoveAlongPatrolRoute.h"

#include "PC_AIController.h"
#include "Actor/PC_PatrolRoute.h"
#include "PC/PC.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTTask_MoveAlongPatrolRoute::UPC_BTTask_MoveAlongPatrolRoute()
{
	NodeName = TEXT("Move Along PatrolRoute Node");
}

EBTNodeResult::Type UPC_BTTask_MoveAlongPatrolRoute::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!ControllingPawn)
		return EBTNodeResult::Failed;

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if (!AIPawn)
		return EBTNodeResult::Failed;

	APC_AIController* AIController = Cast<APC_AIController>(ControllingPawn->GetController());
	if (!AIController)
		return EBTNodeResult::Failed;

	APC_PatrolRoute* PatrolRoute = Cast<APC_PatrolRoute>(AIPawn->GetPatrolRoute());
	if (!PatrolRoute)
		return EBTNodeResult::Failed;

	FVector TargetLocation = PatrolRoute->GetLocationAtSplinePoint();
	EPathFollowingRequestResult::Type PathFollowingRequestResult = AIController->MoveToLocation(TargetLocation, 1.f, false);
	if (PathFollowingRequestResult == EPathFollowingRequestResult::RequestSuccessful)
	{
		UE_LOG(LogPC, Warning, TEXT("MoveTo Request Successful"));
	}
	else
	{
		UE_LOG(LogPC, Warning, TEXT("MoveTo Request Failed or Already Moving"));
	}
	
	return EBTNodeResult::Succeeded;
}
