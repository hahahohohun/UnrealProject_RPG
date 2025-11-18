// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_BattleComponent.h"

#include "MovieSceneFwd.h"
#include "NiagaraFunctionLibrary.h"
#include "PC_ActionComponent.h"
#include "PC_StatComponent.h"
#include "Engine/DamageEvents.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "PC/PC.h"
#include "PC/Battle/PC_NormalAttackDamageType.h"
#include "PC/Character/PC_BaseCharacter.h"
#include "PC/Character/PC_PlayableCharaceter.h"
#include "PC/Character/Controller/PC_PlayerController.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/Interface/PC_CharacterAIInterface.h"
#include "PC/Interface/PC_PlayerCharacterInterface.h"
#include "PC/SkillObject/PC_SkillObject.h"
#include "PC/Utills/PC_GameUtill.h"

// Sets default values for this component's properties
UPC_BattleComponent::UPC_BattleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	TraceInterval = 0.01f;

}

void UPC_BattleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Tick_TraceWeapon(DeltaTime);
	Tick_Assassinate(DeltaTime);
}

void UPC_BattleComponent::Tick_Assassinate(float DeltaTime)
{
	if(IPC_PlayerCharacterInterface* PlayerCharacterInterface = Cast<IPC_PlayerCharacterInterface>(GetOwner()))
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
		if(!PlayerController)
			return;

		UPC_ActionComponent* ActComp = PlayerCharacterInterface->GetActionComponent();
		check(ActComp);
		
		if(ActComp->IsAssassinating)
		{
			if(AssassinateTarget.IsValid())
			{
				AssassinatingElapsedTime += DeltaTime;
				if(AssassinatingElapsedTime < 0.2f)
				{
					const FVector PlayerLocation = OwnerCharacter->GetActorLocation();
					const FVector TargetLocation = AssassinateTarget->GetActorLocation();

					const FVector LookAtLocation = (TargetLocation - PlayerLocation).GetSafeNormal2D();

					const FVector ProperPlayerLocation = TargetLocation - LookAtLocation * 85.f;
					const FVector NewLocation = FMath::VInterpTo(PlayerLocation,ProperPlayerLocation, DeltaTime, 1);

					OwnerCharacter->SetActorLocation(NewLocation);
				}
			}
		}
	}
}

void UPC_BattleComponent::Tick_TraceWeapon(float DeltaTime)
{
		if (!bTracing)
		return;

	TraceElapsedTime += DeltaTime;
	if (TraceElapsedTime < TraceInterval)
		return;

	TraceElapsedTime = 0.f;

	APC_BaseCharacter* Character = Cast<APC_BaseCharacter>(GetOwner());
	if (!Character)
		return;

	const USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
		return;

	const UWorld* World = GetWorld();
	if (!World)
		return;

	FVector CurStartBoneLocation = FVector::ZeroVector;
	FVector CurEndBoneLocation = FVector::ZeroVector;
	//1:28
	if (HasWeapon())
	{
		if (IPC_CharacterInterface * Interface = Cast<IPC_CharacterInterface>(GetOwner()))
		{
			UStaticMeshComponent* WeaponMesh = bTraceRightWeapon? Interface->GetWeapon_R_StaticMeshComponent() : Interface->GetWeapon_L_StaticMeshComponent();
			check(WeaponMesh);

			CurStartBoneLocation = WeaponMesh->GetSocketLocation(TraceStartBoneName);
			CurEndBoneLocation = WeaponMesh->GetSocketLocation(TraceEndBoneName);
		}
	}
	else
	{
		CurStartBoneLocation = Mesh->GetBoneLocation(TraceStartBoneName);
		CurEndBoneLocation = Mesh->GetBoneLocation(TraceEndBoneName);
	}

	// Trace 할 라인들 모음
	TArray<TPair<FVector, FVector>> TraceLines;
	TraceLines.Emplace(PrevStartBoneLocation, CurStartBoneLocation);  
	TraceLines.Emplace(PrevEndBoneLocation, CurEndBoneLocation);
	TraceLines.Emplace(PrevStartBoneLocation, CurEndBoneLocation);    
	TraceLines.Emplace(PrevEndBoneLocation, CurStartBoneLocation);    
	TraceLines.Emplace(CurStartBoneLocation, CurEndBoneLocation);

	int32 SegmentCount = 3;
	
	// 💡 추가: 분할 점 기반 연결선
	for (int32 i = 1; i < SegmentCount; ++i)
	{
		const float Alpha = static_cast<float>(i) / SegmentCount;

		const FVector PrevMid = FMath::Lerp(PrevStartBoneLocation, PrevEndBoneLocation, Alpha);
		const FVector CurrMid = FMath::Lerp(CurStartBoneLocation, CurEndBoneLocation, Alpha);

		TraceLines.Emplace(PrevMid, CurrMid);
	}
	
	//TODO 공격별로 히트 효과 여부 처리
	bool ShouldHitAction = false;
	if(ActionComponent)
		ShouldHitAction = ActionComponent->IsLastAttack();
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	//FPC_GameUtil::CameraShake(EPC_CameraShakeMagnitudeType::Weak);

	for (const auto& Line : TraceLines)
	{
		FHitResult HitResult;
		ECollisionChannel CollisionChannel = FPC_GameUtil::GetAttackCollisionChannel(Character->CharacterDataID);
		
		if (World->LineTraceSingleByChannel(HitResult, Line.Key, Line.Value, CollisionChannel, Params))
		{
			AActor* HitActor = HitResult.GetActor();

			if (HitActor && !DamagedActor.Contains(HitActor))
			{
				DamagedActor.Add(HitActor);

				if (APC_BaseCharacter* HitCharacter = Cast<APC_BaseCharacter>(HitActor))
				{
					if(IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(HitActor))
					{
						if(CharacterInterface->IsRolling())
							continue;

						if(CharacterInterface->IsGuarding(HitResult.ImpactPoint))
							continue;
					}
					
					if(ShouldHitAction)
						FPC_GameUtil::PlayStopDilation(this, 0.2f, 0.f);

					const float Damage = bPowerAttack ?  Character->StatComponent->GetTotalStat().PowerAttack
						: Character->StatComponent->GetTotalStat().Attack;

					UE_LOG(LogPC, Log, TEXT("Hit!! %f"), Damage);
					
					//UPC_NormalAttackDamageType DamageEvent;
					//auto AttackType = UPC_NormalAttackDamageType::StaticClass();

					//FNormalAttackDamageEvent DamageEvent;
					//DamageEvent.bPowerAttack = bPowerAttack; // 여기서 세팅
					FNormalAttackDamageEvent DamageEvent;
					DamageEvent.DamageTypeClass = UDamageType::StaticClass(); 
					DamageEvent.bPowerAttack = bPowerAttack;

					HitActor->TakeDamage(Damage, DamageEvent, Character->GetController(), Character);
						
					if (UPC_CharacterDataAsset* HitCharDataAsset = HitCharacter->GetCharacterDataAsset())
					{
						FPC_GameUtil::SpawnEffectAtLocation(GetWorld(), HitCharDataAsset->HitFx, HitResult.ImpactPoint, FRotator::ZeroRotator, 1);
						//SpawnEffect(HitResult.ImpactPoint, HitCharDataAsset->HitFx);
					}
					

					if(APC_PlayableCharaceter* PlayableCharaceter = Cast<APC_PlayableCharaceter>(OwnerCharacter))
					{
						PlayableCharaceter->PlayHitBlurEffect();
					}
	
				}
			}
		}

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		//HitPart가 가능한 몹 체크용(거대보스)
		if(World->LineTraceSingleByObjectType(HitResult, Line.Key, Line.Value, ObjectQueryParams, Params))
		{
			AActor* HitActor = HitResult.GetActor();
			if(HitActor && !DamagedActor.Contains(HitActor))
			{
				//캐릭터 체크
				if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
				{
					DamagedActor.Add(HitActor);

					FPointDamageEvent DamageEvent;
					DamageEvent.DamageTypeClass = UPC_NormalAttackDamageType::StaticClass();
					DamageEvent.HitInfo = HitResult;

					const float Damage = bPowerAttack ?  Character->StatComponent->GetTotalStat().PowerAttack
						: Character->StatComponent->GetTotalStat().Attack;

					HitActor->TakeDamage(Damage, DamageEvent, Character->GetController(), Character);

					FPC_GameUtil::CameraShake(EPC_CameraShakeMagnitudeType::Weak);
					FPC_GameUtil::PlayStopDilation(this, 0.1f, 0.f);
				}
			}
		}
		
		if(FPC_GameUtil::IsDebugDrawing(OwnerCharacter.Get()))
		{
			DrawDebugLine(World, Line.Key, Line.Value, FColor::Red, false, 3.f, 0, 1.f);
		}
	}

	// Prev 갱신
	PrevStartBoneLocation = CurStartBoneLocation;
	PrevEndBoneLocation = CurEndBoneLocation;
}


void UPC_BattleComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = CastChecked<ACharacter>(GetOwner());

	IPC_CharacterInterface* CharacterInterface = Cast<IPC_CharacterInterface>(OwnerCharacter);
	check(CharacterInterface);

	UPC_CharacterDataAsset* CharacterData = CharacterInterface->GetCharacterDataAsset();
	check(CharacterData);

	if(IPC_PlayerCharacterInterface* HitPlayerCharacterInterface = Cast<IPC_PlayerCharacterInterface>(OwnerCharacter))
	{
		ActionComponent = HitPlayerCharacterInterface->GetActionComponent();
	}
	
	TArray<FPC_WeaponData>& WeaponDatas = CharacterData->WeaponIds;

	for(int32 i = 0; i< WeaponDatas.Num(); ++i)
	{
		Weapons.Add(WeaponDatas[i]);
	}

	CurWeaponIdx = -1;
	SwapWeapon();
}

void UPC_BattleComponent::StartTraceWithWeapon(bool bRight, bool bPowerAtk)
{
	bTracing = true;
	bTraceRightWeapon = bRight;
	bPowerAttack = bPowerAtk;
	
	FPC_WeaponTableRow* WeaponTableRow = bRight ? Weapon_R_TableRow : Weapon_L_TableRow;
	if(!WeaponTableRow)
		return;
	
	TraceStartBoneName = WeaponTableRow->TraceStartSocketName;
	TraceEndBoneName = WeaponTableRow->TraceEndSocketName;

	if (IPC_CharacterInterface* Interface = Cast<IPC_CharacterInterface>(GetOwner()))
	{
		UStaticMeshComponent* WeaponMesh = bTraceRightWeapon? Interface->GetWeapon_R_StaticMeshComponent()
		: Interface->GetWeapon_L_StaticMeshComponent();
		
		check(WeaponMesh);

		PrevStartBoneLocation = WeaponMesh->GetSocketLocation(TraceStartBoneName);
		PrevEndBoneLocation = WeaponMesh->GetSocketLocation(TraceEndBoneName);
	}
}

void UPC_BattleComponent::StartTrace(FName InTraceStartBoneName, FName InTraceEndBoneName)
{
	bTracing = true;
	
	TraceStartBoneName = InTraceStartBoneName;
	TraceEndBoneName = InTraceEndBoneName;
	
	const ACharacter* Character = CastChecked<ACharacter>(GetOwner());
	const USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh();
	check(SkeletalMeshComponent);
	
	PrevStartBoneLocation = SkeletalMeshComponent->GetSocketLocation(TraceStartBoneName);
	PrevEndBoneLocation = SkeletalMeshComponent->GetSocketLocation(TraceEndBoneName);
}


void UPC_BattleComponent::SwapWeapon()
{
	if(!CanSwapWeapon())
		return;
	
	CurWeaponIdx++;

	const int32  num = Weapons.Num();

	if (num <= 0)
	{
		return;
	}
	
	if(CurWeaponIdx >= num)
		CurWeaponIdx = 0;
	
	const FPC_WeaponData& weaponIds = Weapons[CurWeaponIdx];

	CharacterStanceType = static_cast<EPC_CharacterStanceType>(CurWeaponIdx); 

	UnEquipWeapon();
	EquipWeapon(weaponIds.WeaponId_L, false);
	EquipWeapon(weaponIds.WeaponId_R, true);
}

bool UPC_BattleComponent::CanSwapWeapon()
{
	//TODO SKILL, Special Action
	return true;
}

void UPC_BattleComponent::EquipWeapon(uint8 InWeaponId, bool bRightHand)
{
	if (bRightHand)
	{
		Weapon_R_TableRow = FPC_GameUtil::GetWeaponData(InWeaponId);
	}
	else
	{
		Weapon_L_TableRow = FPC_GameUtil::GetWeaponData(InWeaponId);
	}

	if((bRightHand && !Weapon_R_TableRow) || !bRightHand && !Weapon_L_TableRow)
	{
		UnEquipWeapon();
		return;
	}
	
	if (IPC_CharacterInterface* Interface = Cast<IPC_CharacterInterface>(GetOwner()))
	{
		UPC_CharacterDataAsset* CharacterData = Interface->GetCharacterDataAsset();
		check(CharacterData);

		FPC_WeaponTableRow* WeaponTableRow = bRightHand ? Weapon_R_TableRow : Weapon_L_TableRow;
		FName WeaponSocketName = bRightHand ? CharacterData->WeaponSocketName_R : CharacterData->WeaponSocketName_L;

		const ACharacter* Character = CastChecked<ACharacter>(GetOwner());
		USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh();
		check(SkeletalMeshComponent);
		
		UStaticMeshComponent* WeaponStaticMeshComponent = bRightHand?
			Interface->GetWeapon_R_StaticMeshComponent() : Interface->GetWeapon_L_StaticMeshComponent();
		
		check(WeaponStaticMeshComponent);

		if(WeaponTableRow->WeaponSparkFX_Niagara)
			Weapon_Spark_Effect = WeaponTableRow->WeaponSparkFX_Niagara;
		
		WeaponStaticMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		WeaponStaticMeshComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
		
		WeaponStaticMeshComponent->SetRelativeLocation(WeaponTableRow->RelativePos);
		WeaponStaticMeshComponent->SetRelativeRotation(WeaponTableRow->RelativeRot);

		//크기
		WeaponStaticMeshComponent->SetRelativeScale3D(WeaponTableRow->RelativeScale);
		
		WeaponStaticMeshComponent->SetStaticMesh(WeaponTableRow->WeaponMesh);
		WeaponStaticMeshComponent->SetVisibility(true);
	}
}

void UPC_BattleComponent::UnEquipWeapon()
{
	Weapon_L_TableRow = nullptr;
	Weapon_R_TableRow = nullptr;

	if(IPC_CharacterInterface* Interface = Cast<IPC_CharacterInterface>(GetOwner()))
	{
		UStaticMeshComponent* Weapon_L_StaticMeshComponent = Interface->GetWeapon_L_StaticMeshComponent();
		check(Weapon_L_StaticMeshComponent);

		UStaticMeshComponent* Weapon_R_StaticMeshComponent = Interface->GetWeapon_R_StaticMeshComponent();
		check(Weapon_R_StaticMeshComponent);

		Weapon_L_StaticMeshComponent->SetStaticMesh(nullptr);
		Weapon_L_StaticMeshComponent->SetVisibility(false);

		Weapon_R_StaticMeshComponent->SetStaticMesh(nullptr);
		Weapon_R_StaticMeshComponent->SetVisibility(false);
	}
}

bool UPC_BattleComponent::HasWeapon()
{
	if (Weapon_L_TableRow || Weapon_R_TableRow)
		return true;

	return false;
}

void UPC_BattleComponent::ShowWeaponSparkEffect(bool bStart, bool bRight)
{
	if (HasWeapon() == false)
		return;

	// NiagaraComponent가 없다면 생성
	if (!WeaponSparkNiagara)
	{
		if (Weapon_Spark_Effect != nullptr)
		{
			WeaponSparkNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
				Weapon_Spark_Effect,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				false   // AutoDestroy NO (재활용)
			);

			WeaponSparkNiagara->Deactivate(); // 기본은 꺼진 상태
		}
		else
		{
			return;
		}
	}

	// 소켓명 결정
	FName SocketName = bRight ? TEXT("TraceStart") : TEXT("TraceStart");

	// 소켓 Transform 가져오기
	FTransform SocketTransform = FPC_GameUtil::GetSocketTransform(GetOwner(), SocketName);

	if (!SocketTransform.Equals(FTransform()))
	{
		WeaponSparkNiagara->SetWorldTransform(SocketTransform);
	}

	// ON / OFF
	if (bStart)
	{
		WeaponSparkNiagara->Activate(true);
	}
	else
	{
		WeaponSparkNiagara->Deactivate();
	}
}

void UPC_BattleComponent::FireProjectile(bool IsPressed)
{
	if (!IsPressed)
		return;
	
	if (CharacterStanceType == EPC_CharacterStanceType::Staff)
	{
		const APlayerController* PlayerController = CastChecked<APlayerController>(OwnerCharacter->GetController());
	
		USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
		check(SkeletalMeshComponent);

		FVector Location = SkeletalMeshComponent->GetSocketLocation(TEXT("hand_l"));
		FRotator Rotation = PlayerController->GetControlRotation();
	
		FTransform Transform;
		Transform.SetLocation(Location);
		Transform.SetRotation(Rotation.Quaternion());

		//Beginplay 바로 호출
		//GetWorld()->SpawnActor();

		//BeginPlay 호출안함. spawn 완료 위해서는 명시적으로 FinishSpawning 함수 호출해줘야함
		//GetWorld()->SpawnActorDeferred()
		//FinishSpawning 함수 호출이 되면 그때 beginplay 가 호출이 됨

		APC_PlayableCharaceter* PlayableCharacter = Cast<APC_PlayableCharaceter>(OwnerCharacter);
		check(PlayableCharacter);
	
		APC_SkillObject* SkillObject = GetWorld()->SpawnActorDeferred<APC_SkillObject>(PlayableCharacter->ProjectileClass, Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		SkillObject->OwnerCharacter = OwnerCharacter.Get();
		SkillObject->FinishSpawning(Transform);
	}
}

AActor* UPC_BattleComponent::GetAssassinateTarget() const
{
	IPC_PlayerCharacterInterface* Interface = Cast<IPC_PlayerCharacterInterface>(GetOwner());
	check(Interface);

	UPC_InteractionComponent* InteractionComponent = Interface->GetInteractionComponent();
	check(InteractionComponent);

	return InteractionComponent->GetAssassinateTarget();
}

bool UPC_BattleComponent::TryAssassinate()
{
	AActor* TargetActor = GetAssassinateTarget();
	if(!TargetActor)
		return false;

	Assassinate(TargetActor);

	return true;
}

void UPC_BattleComponent::Assassinate(AActor* Target)
{
	AssassinateTarget = Cast<ACharacter>(Target);
	AssassinatingElapsedTime = 0;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = AssassinateTarget->GetActorLocation();

	FVector LookAtRot = (TargetLocation - PlayerLocation).GetSafeNormal2D();
	const FRotator NewRot = LookAtRot.Rotation();

	//각도 보정
	OwnerCharacter->SetActorRotation(NewRot);
	AssassinateTarget->SetActorRotation(NewRot);
	
	IPC_CharacterInterface* Interface = Cast<IPC_CharacterInterface>(AssassinateTarget);
	check(Interface);

	UPC_BattleComponent* BattleComponent = Interface->GetBattleComponent();
	check(BattleComponent);

	UPC_StatComponent* StatComponent = Interface->GetStatComponent();
	check(StatComponent);

	BattleComponent->IsAssassinated = true;

	float MaxHP = StatComponent->MaxHp;
	StatComponent->ApplyDamage(MaxHP, OwnerCharacter.Get(), true);
}

void UPC_BattleComponent::EndTrace()
{
	DamagedActor.Empty();
	bTracing = false;
	bPowerAttack = false;
	TraceElapsedTime = 0.f;

	FPC_GameUtil::AddOnScreenDebugMessage("end trace!!!");
}
