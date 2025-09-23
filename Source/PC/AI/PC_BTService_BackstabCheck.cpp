// Fill out your copyright notice in the Description page of Project Settings.
#include "PC_BTService_BackstabCheck.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

//ex) 암살할때 거리 체크
UPC_BTService_BackstabCheck::UPC_BTService_BackstabCheck()
{
	NodeName = TEXT("BackstabCheck Node");
	bCreateNodeInstance = true;

}

void UPC_BTService_BackstabCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	bAttackIndicator = false;
	
	ACharacter* ControllingCharacter = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if(ControllingCharacter)
	{
		//플레이어 발견 못했고,
		bool bIsAware = OwnerComp.GetBlackboardComponent()->GetValueAsBool(TEXT("bIsAware"));
		if(!bIsAware)
		{
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
			{
				//BlackboardPtr->SetValueAsObject(TEXT("PlayerActor"), PlayerPawn);

				const float Dist2D = FVector::Dist2D(ControllingCharacter->GetActorLocation(), PlayerPawn->GetActorLocation());
				bAttackIndicator = (Dist2D <= Range);
			}
			
			//AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("PlayerActor")));
			//if (Player)
			//{
			//	const float Dist2D = FVector::Dist2D(ControllingCharacter->GetActorLocation(), Player->GetActorLocation());
			//	bAttackIndicator = (Dist2D <= Range);
			//}
		}
	}
	
	if(IPC_CharacterWidgetInterface* Character = Cast<IPC_CharacterWidgetInterface>(ControllingCharacter))
	{
		Character->OnAttackIndicator(bAttackIndicator);
	}

	//밑에는 필요할지 모르겠음
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("IsAttackIndicator"), bAttackIndicator);
}
