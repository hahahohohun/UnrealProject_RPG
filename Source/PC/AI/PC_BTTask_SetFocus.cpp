// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTTask_SetFocus.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPC_BTTask_SetFocus::UPC_BTTask_SetFocus()
{
	NodeName = TEXT("Set Focus Node");
}

//EQS를 활용한 게걸음할때 타겟을 바라보게 하는 Task Node
EBTNodeResult::Type UPC_BTTask_SetFocus::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	//캐릭터 가져와서 그 캐릭터 이동방향에 따른 회전 여부 설정
	ACharacter* ControllingCharacter = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if(!ControllingCharacter)
		return EBTNodeResult::Failed;

	//포커스한다면 false로하여 몸이 회전하지 않도록
	ControllingCharacter->GetCharacterMovement()->bOrientRotationToMovement = !bFocus;
	
	return EBTNodeResult::Succeeded;
}