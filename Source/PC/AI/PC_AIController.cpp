// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_AIController.h"

#include "Actor/PC_PatrolRoute.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "PC/Character/PC_NonPlayableCharacter.h"
#include "PC/Utills/PC_GameUtill.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

APC_AIController::APC_AIController()
{
	PrimaryActorTick.bCanEverTick = true;

	UAIPerceptionComponent* AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	SightSense = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	HearingSense = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	DamageSense = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

	PerceptionComponent->ConfigureSense(*SightSense);
	PerceptionComponent->ConfigureSense(*HearingSense);
	PerceptionComponent->ConfigureSense(*DamageSense);

	//우선순위
	PerceptionComponent->SetDominantSense(SightSense->GetSenseImplementation());

	
	PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ThisClass::OnPerceptionUpdate);
}

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

void APC_AIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.Code == EPathFollowingResult::Success)
	{
		if (IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn()))
		{
			if (APC_PatrolRoute* PatrolRoute = Cast<APC_PatrolRoute>(AIPawn->GetPatrolRoute()))
			{
				PatrolRoute->IncrementIndex();
			}
		}
	}
}

FPC_EnemyTableRow* APC_AIController::GetEnemyData()
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	check(AIPawn);

	FPC_EnemyTableRow* EnemyTableRow = AIPawn->GetEnemyData();
	check(EnemyTableRow);

	return EnemyTableRow;
	
}

void APC_AIController::OnPerceptionUpdate(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* UpdatedActor : UpdatedActors)
	{
		if (GetAIStimulus(UpdatedActor, EPC_AISenseType::Sight).WasSuccessfullySensed())
		{
			HandleSensedSight(UpdatedActor);
			break;
		}

		FAIStimulus Stimulus = GetAIStimulus(UpdatedActor, EPC_AISenseType::Hearing);
		if (Stimulus.WasSuccessfullySensed())
		{
			//자극이 발생한 위치
			HandleSensedHearing(UpdatedActor, Stimulus.StimulusLocation);
			break;
		}
		
		if (GetAIStimulus(UpdatedActor, EPC_AISenseType::Damage).WasSuccessfullySensed())
		{
			HandleSensedDamage(UpdatedActor);
			break;
		}
	}
}

void APC_AIController::SetupSenseConfig()
{
	const FPC_EnemyTableRow* EnemyTableRow = GetEnemyData();
	ensure(EnemyTableRow);

	SightSense->SightRadius = EnemyTableRow->SightRadius;
	SightSense->LoseSightRadius = EnemyTableRow->LoseSightRadius;
	SightSense->PeripheralVisionAngleDegrees = EnemyTableRow->SightAngle;
	SightSense->SetMaxAge(5.f);
	SightSense->DetectionByAffiliation.bDetectEnemies = true;

	HearingSense->HearingRange = 1500.f;
	HearingSense->DetectionByAffiliation.bDetectEnemies = true;
	HearingSense->SetMaxAge(5.f);

	DamageSense->SetMaxAge(2.f);

	//
	PerceptionComponent->ConfigureSense(*SightSense);
	PerceptionComponent->ConfigureSense(*HearingSense);
	PerceptionComponent->ConfigureSense(*DamageSense);
}

void APC_AIController::HandleSensedSight(AActor* InActor)
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	AIPawn->ChangeState(EPC_EnemyStateType::Battle);
	GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), InActor); 
}

void APC_AIController::HandleSensedHearing(AActor* InActor, FVector InLocation)
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	if (AIPawn->GetState() == EPC_EnemyStateType::Battle)
		return;

	AIPawn->ChangeState(EPC_EnemyStateType::Investigating);
	GetBlackboardComponent()->SetValueAsVector(TEXT("InvestigatingPos"), InLocation);
}

void APC_AIController::HandleSensedDamage(AActor* InActor)
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	AIPawn->ChangeState(EPC_EnemyStateType::Battle);
	GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), InActor);
}

void APC_AIController::HandleLoseTarget(AActor* InActor)
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	AIPawn->ChangeState(EPC_EnemyStateType::Patrol);
	GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), nullptr);
}

FAIStimulus APC_AIController::GetAIStimulus(AActor* Actor, EPC_AISenseType AIPerceptionSense)
{
	FActorPerceptionBlueprintInfo ActorPerceptionBlueprintInfo;
	FAIStimulus ResultStimulus;

	GetPerceptionComponent()->GetActorsPerception(Actor, ActorPerceptionBlueprintInfo);

	TSubclassOf<UAISense> QuerySenseClass;
	switch (AIPerceptionSense)
	{
	case EPC_AISenseType::Sight:
		QuerySenseClass = UAISense_Sight::StaticClass();
		break;
	case EPC_AISenseType::Hearing:
		QuerySenseClass = UAISense_Hearing::StaticClass();
		break;
	case EPC_AISenseType::Damage:
		QuerySenseClass = UAISense_Damage::StaticClass();
		break;
	default:
		break;
	}

	TSubclassOf<UAISense> LastSensedStimulusClass;

	for (const FAIStimulus& AIStimulus : ActorPerceptionBlueprintInfo.LastSensedStimuli)
	{
		LastSensedStimulusClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, AIStimulus);
		
		if (QuerySenseClass == LastSensedStimulusClass)
		{
			ResultStimulus = AIStimulus;
			return ResultStimulus;
		}
	}
	
	return ResultStimulus;
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

	SetupSenseConfig();
	RunAI();
}

void APC_AIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AActor* CurrentActor = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (!CurrentActor)
		return;

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	const FAIStimulus SightStimulus = GetAIStimulus(CurrentActor, EPC_AISenseType::Sight);
	const FAIStimulus DamageStimulus = GetAIStimulus(CurrentActor, EPC_AISenseType::Damage);
	
	const bool bLostSight = !SightStimulus.WasSuccessfullySensed() && SightStimulus.IsExpired();
	const bool bDamageExpired = !DamageStimulus.IsValid() || DamageStimulus.IsExpired();

	if (bLostSight && bDamageExpired)
	{
		HandleLoseTarget(CurrentActor);
	}
}
