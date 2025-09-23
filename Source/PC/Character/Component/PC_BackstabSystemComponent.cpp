#include "PC_BackstabSystemComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_CharacterInterface.h"
#include "PC/Interface/PC_CharacterWidgetInterface.h"

UPC_BackstabSystemComponent::UPC_BackstabSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPC_BackstabSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AttackRangeSqur = AttackRange * AttackRange;
}

bool UPC_BackstabSystemComponent::ExecuteBackstab()
{
	bool result = false;
	if(BackstabTarget)
	{
		if(ACharacter* Character = Cast<ACharacter>(BackstabTarget))
		{
			if(IPC_CharacterAIInterface* CharacterAIInterface = Cast<IPC_CharacterAIInterface>(Character))
			{
				CharacterAIInterface->SetDeadType(EPC_DeadType::Backstab);

				if (ACharacter* Victim = Cast<ACharacter>(BackstabTarget))
				{
					// 공격자 → 피격자 방향 (즉, Victim이 공격자를 바라보는 방향)
					FVector ToAttacker = (GetOwner()->GetActorLocation() - Victim->GetActorLocation()).GetSafeNormal2D();

					if (!ToAttacker.IsNearlyZero())
					{
						// Victim이 공격자를 '등지도록' → 공격자 방향의 반대 벡터
						FVector AwayFromAttacker = -ToAttacker;

						FRotator VictimRot = AwayFromAttacker.Rotation();
						VictimRot.Pitch = 0.f;
						VictimRot.Roll = 0.f;

						Victim->SetActorRotation(VictimRot);
						result = true;
						return result;
					}
				}
			}
		}
	}
	
	return result;
}

void UPC_BackstabSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;
	if (!Owner->IsLocallyControlled()) return; // 클라 전용

	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
	{
		if(Owner == *It)
			continue;

		//TODO 컬라이더를 이용해서 변화가 생겼을때만 갱신하도록
		
		bool OnIndicator = false;
		if(IPC_CharacterInterface* Character = Cast<IPC_CharacterInterface>(*It))
		{
			if(!Character->IsDead())
			{
				if(IPC_CharacterAIInterface* AI = Cast<IPC_CharacterAIInterface>(*It))
				{
					if(AI->GetState() == EPC_EnemyStateType::Investigating
					|| AI->GetState() == EPC_EnemyStateType::Patrol)
					{
						const float DistSq = FVector::DistSquared(It->GetActorLocation(),Owner->GetActorLocation());
						if(DistSq <= AttackRangeSqur)
						{
							OnIndicator = true;
							SetBackstabTarget(*It);
						}
					}
				}
			}
		}
				
		if(IPC_CharacterWidgetInterface* CharacterWidget = Cast<IPC_CharacterWidgetInterface>(*It))
		{
			CharacterWidget->OnAttackIndicator(OnIndicator);
		}
	}
}

void UPC_BackstabSystemComponent::BackstabOn()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;
	if (!Owner->IsLocallyControlled()) return; // 클라 전용

	//암살 가능한 상태인지 체크,
	bool isBackstab = false;
	if(BackstabTarget)
	{
		//거리 한번 더 체크
		const float DistSq = FVector::DistSquared(BackstabTarget->GetActorLocation(),Owner->GetActorLocation());
		if(DistSq <= AttackRangeSqur)
		{
			isBackstab = true;
		}
	}
	//if (APawn* Target = FindTarget())
	//{
	//	if(AAIController* Owner = Cast<AAIController>(Target->GetController()))
	//	{
	//		if(Owner->GetBlackboardComponent()->GetValueAsBool("IsAttackIndicator"))
	//		{
	//			SetBackstabTarget(Target);
	//			isBackstab = true;
	//		}
	//	}
	//}
	
	SetBackstabOnMode(isBackstab);
}

void UPC_BackstabSystemComponent::SetBackstabTarget(APawn* InActor)
{
	if(!InActor)
		return;

	BackstabTarget = InActor;
	
	if(IPC_CharacterWidgetInterface* Character = Cast<IPC_CharacterWidgetInterface>(BackstabTarget))
	{
		Character->OnAttackIndicator(true);
	}
}

void UPC_BackstabSystemComponent::ClearTarget()
{
	if (IPC_CharacterWidgetInterface* Character = Cast<IPC_CharacterWidgetInterface>(BackstabTarget))
	{
		Character->OnAttackIndicator(false);
	}
	
	BackstabTarget = nullptr;
}

void UPC_BackstabSystemComponent::SetBackstabOnMode(bool bEnable)
{
	bBackstabOnMode = bEnable;
	if (!bBackstabOnMode)
	{
		ClearTarget();
	}
}

bool UPC_BackstabSystemComponent::IsBackstabOnMode() const
{
	return bBackstabOnMode;
}

TObjectPtr<APawn> UPC_BackstabSystemComponent::GetBackstabTarget() const
{
	return BackstabTarget;
}


