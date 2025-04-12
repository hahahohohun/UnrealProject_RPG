// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "PC/Character/PC_NonPlayableCharacter.h"
#include "PC/Utills/PC_GameUtill.h"

void APC_AIController::RunAI()
{
	UBlackboardComponent* BlackboardPtr = GetBlackboardComponent();
	//사용준비
	if (UseBlackboard(BBAsset, BlackboardPtr))
	{
		bool RunReseult = RunBehaviorTree(BTAsset);
		ensure(RunReseult);
	}
}

void APC_AIController::StopAI()
{
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}
}

void APC_AIController::OnPossess(APawn* Possessed)
{
	Super::OnPossess(Possessed);

	if (APC_NonPlayableCharacter* NonPlayableCharacter = Cast<APC_NonPlayableCharacter>(GetPawn()))
	{
		if (const FPC_EnemyTableRow* EnemyTableRow = FPC_GameUtil::GetEnemyData(NonPlayableCharacter->CharacterType))
		{
			BBAsset = EnemyTableRow->BlackBoard;
			BTAsset = EnemyTableRow->BehaviorTree;
		}
	}

	RunAI();
}
