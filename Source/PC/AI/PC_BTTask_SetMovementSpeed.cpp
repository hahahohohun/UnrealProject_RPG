// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTTask_SetMovementSpeed.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTTask_SetMovementSpeed::UPC_BTTask_SetMovementSpeed()
{
	NodeName = TEXT("Set Movement Speed");
}

EBTNodeResult::Type UPC_BTTask_SetMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Reulst = Super::ExecuteTask(OwnerComp, NodeMemory);

	ACharacter* ControllingCharacter = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if(!ControllingCharacter)
	{
		return EBTNodeResult::Failed;
	}

	IPC_CharacterAIInterface* CharacterAIInterface = Cast<IPC_CharacterAIInterface>(ControllingCharacter);
	if(!CharacterAIInterface)
		return EBTNodeResult::Failed;

	FPC_EnemyTableRow* EnemyTableRow = CharacterAIInterface->GetEnemyData();
	ensure(EnemyTableRow);

	float MoveSpeed = 0.f;
	switch (MovementType)
	{
		case EPC_MovementType::Walk:
			MoveSpeed = EnemyTableRow->MovementSpeed_Walk;
			break;
		case EPC_MovementType::Strafe:
			MoveSpeed = EnemyTableRow->MovementSpeed_Strafe;
			break;
		case EPC_MovementType::Run:
			MoveSpeed = EnemyTableRow->MovementSpeed_Run;
			break;
	}

	//speed적용
	ControllingCharacter->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	
	return EBTNodeResult::Succeeded;
}
