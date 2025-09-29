// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTTask_Attack::UPC_BTTask_Attack()
{
	NodeName = TEXT("Attack Node");
}

EBTNodeResult::Type UPC_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* Pawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(Pawn);
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	FAICharacterAttackFinished OnAttackFinished;
	OnAttackFinished.BindLambda(
	[&]()
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	);

	const APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (!Target)
		return EBTNodeResult::Failed;

	FVector TargetLocation = Target->GetActorLocation();
	FVector Location = Pawn->GetActorLocation();

	FVector ToTargetDir = (TargetLocation - Location).GetSafeNormal2D();

	Pawn->SetActorRotation(ToTargetDir.Rotation());

	AIPawn->SetAIAttackFinishDelegate(OnAttackFinished);
	AIPawn->Attack(bLastAttacking);
	
	return EBTNodeResult::InProgress;
}
