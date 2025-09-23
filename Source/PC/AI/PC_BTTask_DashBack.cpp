// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_BTTask_DashBack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTTask_DashBack::UPC_BTTask_DashBack()
{
	NodeName = TEXT("DashBack Node");
}

//플레이어 가까이 왔을때 뒤로 BACK하는 테스크
EBTNodeResult::Type UPC_BTTask_DashBack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	FAICharacterMoveMontageFinished CharacterMoveMontage;
	CharacterMoveMontage.BindLambda([&]
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	});
	
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControllingPawn)
		return EBTNodeResult::Failed;
	
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if(!AIPawn)
		return EBTNodeResult::Failed;

	const APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (!Target)
		return EBTNodeResult::Failed;

	FVector TargetPos = Target->GetActorLocation();
	FVector PawnPos = ControllingPawn->GetActorLocation();
	
	FVector ToTargetDir = (TargetPos - PawnPos).GetSafeNormal2D();

	ControllingPawn->SetActorRotation(ToTargetDir.Rotation());
	AIPawn->SetAIMoveMontageFinishedDelegate(CharacterMoveMontage);
	AIPawn->DashBack();

	Result = EBTNodeResult::InProgress; 
	return Result;
}