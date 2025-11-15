// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_ArcSplinePreviewComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PC/Utills/PC_GameUtill.h"

UPC_ArcSplinePreviewComponent::UPC_ArcSplinePreviewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPC_ArcSplinePreviewComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!ArcSpline)
	{
		ArcSpline = NewObject<USplineComponent>(GetOwner(), TEXT("ArcPreviewSpline"));
		ArcSpline->SetupAttachment(GetOwner()->GetRootComponent());
		ArcSpline->RegisterComponent();
	}
	
	EnsurePool();
	ClearSpline();
}

void UPC_ArcSplinePreviewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SplineMeshPool.Empty();
	Super::EndPlay(EndPlayReason);
}

void UPC_ArcSplinePreviewComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//TickPreview(DeltaTime);
}

void UPC_ArcSplinePreviewComponent::BeginPreview()
{
	StartVel = FVector::ZeroVector;
	EnsurePool();
	bActive = true;
	Acc = 0.f;

	ArcSpline->SetHiddenInGame(false);
	ArcSpline->SetVisibility(true);
}

void UPC_ArcSplinePreviewComponent::EndPreview()
{
	bActive = false;
	ClearSpline();
}

bool UPC_ArcSplinePreviewComponent::UpdateFromStartVelocity(const FVector& Start, const FVector& LaunchVelocity)
{
	FPredictProjectilePathParams Params;
	Params.StartLocation = Start;
	Params.LaunchVelocity = LaunchVelocity;
	Params.bTraceWithCollision = true;
	Params.ProjectileRadius = Config.ProjectileRadius;
	Params.MaxSimTime = Config.MaxSimTime;
	Params.SimFrequency = Config.SimFrequency;
	Params.TraceChannel = ECC_Visibility;

	// OverrideGravityZ 설정 (0이면 UGameplayStatics 내부에서 월드 중력 사용)
	if (Config.OverrideGravityZ != 0.f)
	{
		Params.OverrideGravityZ = Config.OverrideGravityZ;
	}

	for (const auto& Weak : ActorsToIgnore)
	{
		if (AActor* A = Weak.Get())
			Params.ActorsToIgnore.Add(A);
	}
	
	StartVel = FVector::ZeroVector;
	StartVel = LaunchVelocity;
	return UpdateFromPredictParams(Params);
}

bool UPC_ArcSplinePreviewComponent::UpdateFromPredictParams(const struct FPredictProjectilePathParams& InParams)
{
	FPredictProjectilePathResult Result;
	const bool bHit = UGameplayStatics::PredictProjectilePath(this, InParams, Result);

	LastPathPoints.Reset();
	LastPathPoints.Reserve(Result.PathData.Num());
	for (const auto& P : Result.PathData)
	{
		LastPathPoints.Add(P.Location);
	}


	if (FPC_GameUtil::IsDebugDrawing(this))
	{
		for (int32 i=1; i<LastPathPoints.Num(); ++i)
		{
			DrawDebugLine(GetWorld(), LastPathPoints[i-1], LastPathPoints[i], FColor::Cyan, false, 0.f, 0, 2.f);
		}
	}

	BuildSplineFromPoints(LastPathPoints);
	return bHit;
}

void UPC_ArcSplinePreviewComponent::AddActorToIgnore(AActor* Actor)
{
	if (Actor)
		ActorsToIgnore.Add(Actor);
}

void UPC_ArcSplinePreviewComponent::BuildSplineFromPoints(const TArray<FVector>& Points)
{
	const int32 Count = FMath::Min(Config.MaxSplinePoints, Points.Num());
	if (!ArcSpline) return;

	ArcSpline->ClearSplinePoints(false);
	for (int32 i=0; i<Count; ++i)
	{
		ArcSpline->AddSplinePoint(Points[i], ESplineCoordinateSpace::World, false);
	}
	ArcSpline->SetClosedLoop(false, false);
	ArcSpline->UpdateSpline();

	// 스플라인 메시 렌더링
	int32 Used = 0;
	if (Config.SplineMesh && SplineMeshPool.Num() > 0)
	{
		for (int32 i=0; i<Count-1 && Used < SplineMeshPool.Num(); ++i, ++Used)
		{
			USplineMeshComponent* Seg = SplineMeshPool[Used];
			const FVector SPos = ArcSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			const FVector SDir = ArcSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
			const FVector EPos = ArcSpline->GetLocationAtSplinePoint(i+1, ESplineCoordinateSpace::Local);
			const FVector EDir = ArcSpline->GetTangentAtSplinePoint(i+1, ESplineCoordinateSpace::Local);
			
			Seg->SetStartAndEnd(SPos, SDir, EPos, EDir, true);
			Seg->SetStartScale(FVector2D(0.2f));
			Seg->SetEndScale(FVector2D(0.2f));
			Seg->SetHiddenInGame(false);
		}
	}

	// 남는 세그먼트 숨김
	for (int32 i=Used; i<SplineMeshPool.Num(); ++i)
	{
		SplineMeshPool[i]->SetHiddenInGame(true);
	}
}

void UPC_ArcSplinePreviewComponent::ClearSpline()
{
	if (ArcSpline)
	{
		ArcSpline->ClearSplinePoints();
		ArcSpline->UpdateSpline();
		ArcSpline->SetHiddenInGame(true);
		ArcSpline->SetVisibility(false);
	}
	HideAllSegments();
	LastPathPoints.Reset();
}

void UPC_ArcSplinePreviewComponent::HideAllSegments()
{
	for (auto* Seg : SplineMeshPool)
	{
		if (Seg)
			Seg->SetHiddenInGame(true);
	}
}

void UPC_ArcSplinePreviewComponent::EnsurePool()
{
	if (!Config.SplineMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsurePool: Config.SplineMesh is null"));
		return;
	}

	if (SplineMeshPool.Num() >= Config.SplineMeshPoolSize)
		return;

	const int32 Need = Config.SplineMeshPoolSize - SplineMeshPool.Num();
	for (int32 i = 0; i < Need; ++i)
	{
		auto* Seg = NewObject<USplineMeshComponent>(GetOwner());
		Seg->SetMobility(EComponentMobility::Movable);
		Seg->SetStaticMesh(Config.SplineMesh);
		Seg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Seg->AttachToComponent(ArcSpline, FAttachmentTransformRules::KeepRelativeTransform);

		UMaterialInterface* UseMat = Config.SplineMesh->GetMaterial(0);
		if (UseMat)
		{
			Seg->SetMaterial(0, UseMat);
			Seg->MarkRenderStateDirty();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EnsurePool: No valid material for SplineMeshComponent %d"), i);
		}

		Seg->SetHiddenInGame(true);
		Seg->SetForwardAxis(ESplineMeshAxis::Z, true);
		Seg->SetStartScale(FVector2D(1.f, 1.f));
		Seg->SetEndScale(FVector2D(1.f, 1.f));
		Seg->SetStartRoll(0.f);
		Seg->SetEndRoll(0.f);

		Seg->RegisterComponent();
		SplineMeshPool.Add(Seg);
	}
}
