// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_SkillObject.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "PC/Utills/PC_GameUtill.h"

// Sets default values
APC_SkillObject::APC_SkillObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision_Environment = CreateDefaultSubobject<USphereComponent>(TEXT("Collision_Environment"));
	RootComponent = Collision_Environment;
	
	TriggerCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	TriggerCollision->SetupAttachment(RootComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(RootComponent); 
}

// Called when the game starts or when spawned
void APC_SkillObject::BeginPlay()
{
	Super::BeginPlay();
	
	if (!TriggerCollision->OnComponentBeginOverlap.IsAlreadyBound(this, &ThisClass::OnBeginOverlap))
		TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);

	check(Collision_Environment);
	Collision_Environment->SetCollisionProfileName(TEXT("SkillObject_Environment"));
	Collision_Environment->SetSphereRadius(1.f);

	if (!Collision_Environment->OnComponentHit.IsAlreadyBound(this, &ThisClass::OnComponentHit))
		Collision_Environment->OnComponentHit.AddDynamic(this, &ThisClass::OnComponentHit);

	if (BounceCount > 0)
		ProjectileMovementComponent->bShouldBounce = true;
}

// Called every frame
void APC_SkillObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;
	if (ElapsedTime > LifeTime)
	{
		ProcessDestroy();
	}
}

void APC_SkillObject::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FPC_SkillObjectTableRow* SkillObjectTableRow = FPC_GameUtil::GetSkillObjectData(SkillObjectId);
	check(SkillObjectTableRow);
	
	if (ACharacter* HitCharacter = Cast<ACharacter>(OtherActor))
	{
		FDamageEvent DamageEvent;
		HitCharacter->TakeDamage(SkillObjectTableRow->Damage, DamageEvent, OwnerCharacter->GetController(), OwnerCharacter.Get());
	}
	
	ProcessDestroy();
}

void APC_SkillObject::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void APC_SkillObject::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (BounceCount > 0)
	{
		BounceCount--;
	}
	else if (BounceCount == 0)
	{
		ProcessDestroy();
	}
}

void APC_SkillObject::PlaySound()
{
	FVector ActorLocation = GetActorLocation();
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DeSpawnSound, ActorLocation);
}

void APC_SkillObject::PlayFX(FVector InHitLocation)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DespawnFX, GetActorLocation(), GetActorRotation());
	FPC_GameUtil::CameraShake();
}

void APC_SkillObject::ProcessDestroy()
{
	TriggerCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collision_Environment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovementComponent->Deactivate();
	StaticMeshComponent->SetVisibility(false);
	
	PlaySound();
	PlayFX(GetActorLocation());

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		Destroy();
	}), 0.5f, false);
}

