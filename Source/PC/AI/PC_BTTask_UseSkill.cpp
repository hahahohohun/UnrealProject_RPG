// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BTTask_UseSkill.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterInterface.h"

UPC_BTTask_UseSkill::UPC_BTTask_UseSkill()
{
	NodeName = TEXT("UseSkill Node");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UPC_BTTask_UseSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (!ControllingPawn)
		return EBTNodeResult::Failed;

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(ControllingPawn);
	if (!AIPawn)
		return EBTNodeResult::Failed;

	IPC_CharacterInterface* IPCChar = Cast<IPC_CharacterInterface>(ControllingPawn);
	check(IPCChar);

	UPC_SkillComponent* SkillComponent = IPCChar->GetSkillComponent();
	check(SkillComponent);

	BehaviorTreeComponent = &OwnerComp;
	OwenrCharacter = Cast<ACharacter>(ControllingPawn);

	FPC_OnEndSkillDelegate& OnEndSkillDelegate = SkillComponent->OnEndSkillDelegate;

	if(!OnEndSkillDelegate.IsAlreadyBound(this, &ThisClass::OnEndSkill))
	{
		OnEndSkillDelegate.AddDynamic(this, &ThisClass::OnEndSkill);	
	}

	SkillComponent->RequestPlaySkill(SkillId);
	
	return EBTNodeResult::InProgress;
}

void UPC_BTTask_UseSkill::OnEndSkill(uint32 InSkillId)
{
	IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwenrCharacter);
	if(CharacterInterface)
	{
		FPC_OnEndSkillDelegate& OnEndSkillDelegate = CharacterInterface->GetOnEndSkillDelegate();

		if(OnEndSkillDelegate.IsAlreadyBound(this, &ThisClass::OnEndSkill))
		{
			OnEndSkillDelegate.RemoveDynamic(this, &ThisClass::OnEndSkill);
		}
	}

	//종료되는 
	if(InSkillId == SkillId)
	{
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	}
}
