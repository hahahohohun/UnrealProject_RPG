// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_AIController.h"

#include "Actor/PC_PatrolRoute.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "PC/Character/PC_NonPlayableCharacter.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
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
	
	//ensure(BlackboardPtr);
	
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
	IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(GetPawn());
	ensure(CharacterInterface);

	UPC_CrowdControlComponent* CrowdControlComponent = CharacterInterface->GetCrowdControlComponent();
	check(CrowdControlComponent);

	if (CrowdControlComponent->IsCrowdControlled())
		return;

	const EPC_EnemyStateType EnemyState = static_cast<EPC_EnemyStateType>(GetBlackboardComponent()->GetValueAsEnum("State"));
	if(EnemyState == EPC_EnemyStateType::Dead)
		return;
	
	if(AActor* TargetActor = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(TEXT("Target"))))
	{
		if(IPC_CharacterInterface* TargetCharacter = Cast<IPC_CharacterInterface>(TargetActor))
		{
			if(TargetCharacter->IsDead())
			{
				HandleLoseTarget(TargetActor);
				return;
			}
		}
	}
	
	for (AActor* UpdatedActor : UpdatedActors)
	{
		if(IPC_CharacterInterface* TargetCharacter = Cast<IPC_CharacterInterface>(UpdatedActor))
		{
			if(TargetCharacter->IsDead())
				continue;
		}
		
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
	//얼마나 기억할건지
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
	
	if (AIPawn->GetState() == EPC_EnemyStateType::Groggy)
		return;

	AIPawn->RequestChangeState(EPC_EnemyStateType::Battle);
	OnSenseTarget(InActor);
}

void APC_AIController::HandleSensedHearing(AActor* InActor, FVector InLocation)
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	if (AIPawn->GetState() == EPC_EnemyStateType::Battle)
		return;
	
	if (AIPawn->GetState() == EPC_EnemyStateType::Groggy)
		return;

	AIPawn->RequestChangeState(EPC_EnemyStateType::Investigating);
	GetBlackboardComponent()->SetValueAsVector(TEXT("InvestigatingPos"), InLocation);
}

void APC_AIController::HandleSensedDamage(AActor* InActor)
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	if(AIPawn->GetState() == EPC_EnemyStateType::Groggy)
		return;
	
	AIPawn->RequestChangeState(EPC_EnemyStateType::Battle);

	OnSenseTarget(InActor);
}

void APC_AIController::HandleLoseTarget(AActor* InActor)
{
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(GetPawn());
	ensure(CharacterInterface);

	UPC_CrowdControlComponent* CrowdControlComponent = CharacterInterface->GetCrowdControlComponent();
	check(CrowdControlComponent);

	if (CrowdControlComponent->IsCrowdControlled())
		return;

	AIPawn->RequestChangeState(EPC_EnemyStateType::Patrol);
	OnSenseTarget(nullptr);
}

void APC_AIController::OnSenseTarget(AActor* InActor)
{
	AActor* PrevTarget = Cast<ACharacter>(GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if(PrevTarget == InActor)
		return;
	
	GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), InActor);

	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	FPC_EnemyTableRow* EnemyTableRow = AIPawn->GetEnemyData();
	ensure(EnemyTableRow);

	if(EnemyTableRow->IsBoss)
	{
		//플레이어와 조우했다면
		if(IPC_PlayerCharacterInterface* PlayerCharacterInterface = Cast<IPC_PlayerCharacterInterface>(InActor))
		{
			if(InActor)
			{
				PlayerCharacterInterface->OnSensedByBossMonster(Cast<ACharacter>(AIPawn));
			}
		}

		if(!InActor)
		{
			if(IPC_PlayerCharacterInterface* PlayerCharacterInterface = Cast<IPC_PlayerCharacterInterface>(PrevTarget))
			{
				PlayerCharacterInterface->OnSensedByBossMonster(nullptr);
			}
		}
	}
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

void APC_AIController::SetGenericTeamId(const FGenericTeamId& GTeamID)
{
	IGenericTeamAgentInterface::SetGenericTeamId(GTeamID);

	GenericTeamId = GTeamID;
}

FGenericTeamId APC_AIController::GetGenericTeamId() const
{
	return GenericTeamId;
}

void APC_AIController::OnPossess(APawn* Possessed)
{
	Super::OnPossess(Possessed);

	SetGenericTeamId(FGenericTeamId(1));

	if (APC_NonPlayableCharacter* NonPlayableCharacter = Cast<APC_NonPlayableCharacter>(GetPawn()))
	{
		if (const FPC_EnemyTableRow* EnemyTableRow = FPC_GameUtil::GetEnemyData(NonPlayableCharacter->CharacterDataID))
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
	
	IPC_CharacterAIInterface* AIPawn = Cast<IPC_CharacterAIInterface>(GetPawn());
	ensure(AIPawn);

	constexpr float UnderRange = PC_EnemyRange::Under;
	constexpr float NearRange = PC_EnemyRange::NearRange;
	constexpr float MiddleRange = PC_EnemyRange::MiddleRange;
	FVector CurrentActorOffset = GetPawn()->GetActorRotation().Vector() * Cast<ACharacter>(GetPawn())->GetMesh()->GetRelativeScale3D().GetMax() * 50.f;

	if(FPC_GameUtil::IsDebugDrawing(this))
	{
		//if(AIPawn->GetEnemyData()->IsHitPartUnit)
		//{
		//	//매쉬가 앞으로 기울어져있어서 보정처리
		//	FVector Center = GetPawn()->GetActorLocation() + GetPawn()->GetActorRotation().Vector() *
		//		Cast<ACharacter>(GetPawn())->GetMesh()->GetRelativeScale3D().GetMax() * 50.f;
		//	
		//	Center.Z -= Cast<ACharacter>(GetPawn())->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		//	DrawDebugSphere(GetWorld(), Center, NearRange, 16, FColor::Yellow, false, -1, 0, 3.f);
		//	DrawDebugSphere(GetWorld(), Center, MiddleRange, 16, FColor::Purple, false, -1,0, 3.f);
		//}


		if (AIPawn->GetEnemyData()->IsHitPartUnit)
		{
			AActor* BossActor = GetPawn(); // 혹은 AIPawn 등

			FVector Offset = BossActor->GetActorRotation().Vector() *
				Cast<ACharacter>(BossActor)->GetMesh()->GetRelativeScale3D().GetMax() * 50.f;
			
			FPC_GameUtil::DrawProximityAngleDebug(
				BossActor,
				UnderRange,
				NearRange,
				MiddleRange,
				45.f,     // Front 기준 각도(±45도)
				Offset);  // 네가 쓰던 앞쪽 보정
		}
	}
	
	AActor* TargetActor = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (!TargetActor)
		return;
	
	FPC_EnemyTableRow* EnemyTableRow = AIPawn->GetEnemyData();
	ensure(EnemyTableRow);

	const FAIStimulus SightStimulus = GetAIStimulus(TargetActor, EPC_AISenseType::Sight);
	const FAIStimulus DamageStimulus = GetAIStimulus(TargetActor, EPC_AISenseType::Damage);
	
	const bool bLostSight = !SightStimulus.WasSuccessfullySensed() && SightStimulus.IsExpired();
	const bool bDamageExpired = !DamageStimulus.IsValid() || DamageStimulus.IsExpired();

	if (bLostSight && bDamageExpired)
	{
		HandleLoseTarget(TargetActor);
	}

	if(EnemyTableRow->IsHitPartUnit)
	{
		EPC_ProximityType TargetProximity = FPC_GameUtil::GetTargetProximity(
			TargetActor, GetPawn(),UnderRange, NearRange, MiddleRange, CurrentActorOffset);

		GetBlackboardComponent()->SetValueAsEnum(TEXT("TargetProximityType"), static_cast<uint8>(TargetProximity));

		if (FPC_GameUtil::IsDebugDrawing(this))
        {
            FVector CurrentLocation = GetPawn()->GetActorLocation() + CurrentActorOffset;
            FVector TargetLocation  = TargetActor->GetActorLocation();
        
            FColor DrawColor = FColor::White;
        
            switch (TargetProximity)
            {
            case EPC_ProximityType::Under: DrawColor = FColor::Black;     break;
            case EPC_ProximityType::Near_l: DrawColor = FColor::Red;      break;
            case EPC_ProximityType::Near_r: DrawColor = FColor::Green;    break;
            case EPC_ProximityType::Front:  DrawColor = FColor::Yellow;   break;
            case EPC_ProximityType::Back:   DrawColor = FColor::Blue;     break;
            case EPC_ProximityType::Left:   DrawColor = FColor::Cyan;     break;
            case EPC_ProximityType::Right:  DrawColor = FColor::Magenta;  break;
            case EPC_ProximityType::Far:    DrawColor = FColor::Silver;   break;
            default: break;
            }
        
            DrawDebugLine(GetWorld(), CurrentLocation, TargetLocation, DrawColor, false, -1, 0, 5.f);
        
            const FRotator CurrentRot = GetPawn()->GetActorRotation();
            const FRotator TargetRot  = (TargetLocation - CurrentLocation).Rotation();
            const float YawDiff = FMath::UnwindDegrees(TargetRot.Yaw - CurrentRot.Yaw);
        
            const FString ProximityName = UEnum::GetValueAsString(TargetProximity);
            const FString DebugText = FString::Printf(TEXT("%s (YawDiff: %.1f°)"), *ProximityName, YawDiff);
        
            DrawDebugString(
                GetWorld(),
                TargetLocation + FVector(0, 0, 200),
                DebugText,
                nullptr, DrawColor, 0.f, true, 3.0f
            );
        }

	}
}
