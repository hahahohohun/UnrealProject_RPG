#include "PC_BTTask_SetState.h"

#include "AIController.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTTask_SetState::UPC_BTTask_SetState()
{
	NodeName = TEXT("BTTask SetState");
}

EBTNodeResult::Type UPC_BTTask_SetState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!ControllingPawn)
	{
		return EBTNodeResult::Failed;
	}

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	AIPawn->ChangeState(StateType);
	
	return EBTNodeResult::Succeeded;
	
}