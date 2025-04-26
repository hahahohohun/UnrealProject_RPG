// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTTask_TurnToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PC/Interface/PC_CharacterAIInterface.h"

UPC_BTTask_TurnToTarget::UPC_BTTask_TurnToTarget()
{
	NodeName = TEXT("TurnToTarget");
	bNotifyTick = true;
}

EBTNodeResult::Type UPC_BTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	const APawn* TargetPawn = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);

	if (!TargetPawn || !ControllingPawn || !AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	FVector LookVector = TargetPawn->GetActorLocation() - ControllingPawn->GetActorLocation();
	LookVector.Z = 0.0f;
	const FRotator TargetRot = FRotationMatrix::MakeFromX(LookVector).Rotator();
	const FRotator CurrentRot = ControllingPawn->GetActorRotation();

	float YawDiff = FMath::UnwindDegrees(TargetRot.Yaw - CurrentRot.Yaw);

	if (YawDiff > 45.f && YawDiff < 135.f)
	{
		AIPawn->TurnInPlace(90.f);
	}
	else if (YawDiff >= 135.f)
	{
		AIPawn->TurnInPlace(180.f);
	}
	else if (YawDiff < -45.f && YawDiff > -135.f)
	{
		AIPawn->TurnInPlace(-90.f);
	}
	else if (YawDiff < -135.0f)
	{
		AIPawn->TurnInPlace(-180.f);
	}
	else
	{
		Result = EBTNodeResult::Succeeded;
		return Result;
	}

	FAICharacterAttackFinished FaiCharacterTurnFinishDelegate;
	FaiCharacterTurnFinishDelegate.BindLambda(
		[&]
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	);

	AIPawn->SetAITurnFinishDelegate(FaiCharacterTurnFinishDelegate);
	
	Result = EBTNodeResult::InProgress;
	return Result;
}
