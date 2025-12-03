#include "PC_AfterImageActor.h"

APC_AfterImageActor::APC_AfterImageActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PoseMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseMesh"));
	SetRootComponent(PoseMesh);

	Duration = 0.5f;
	ElapsedTime = 0.0f;

	PoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void APC_AfterImageActor::BeginPlay()
{
	Super::BeginPlay();
}

void APC_AfterImageActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(!DynamicMaterialInstance || Duration <= 0.0f)
		return;

	//10 -> 0.
	ElapsedTime += DeltaTime;
	float Alpha = FMath::Clamp(1.0f -(ElapsedTime / Duration), 0.0f, 1.0f);
	Alpha *= 10.f;
	
	//캐릭터 오버레이, 머테리얼 인스턴스
	DynamicMaterialInstance->SetScalarParameterValue(TEXT("Boost Emssive"), Alpha); 
	if(ElapsedTime >= Duration)
	{
		Destroy();
	}
}

void APC_AfterImageActor::InitFromMesh(USkeletalMeshComponent* SourceMesh)
{
	if(!SourceMesh)
	{
		return;
	}

	PoseMesh->SetSkeletalMesh(SourceMesh->SkeletalMesh);

	//포즈 복사
	PoseMesh->CopyPoseFromSkeletalComponent(SourceMesh);

	SetActorTransform(SourceMesh->GetComponentTransform());

	if(AfterImageMaterial)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(AfterImageMaterial, this);
		for (int32 Index = 0; Index < PoseMesh->GetNumMaterials(); ++Index)
		{
			PoseMesh->SetMaterial(Index, DynamicMaterialInstance);
		}
	}
}



