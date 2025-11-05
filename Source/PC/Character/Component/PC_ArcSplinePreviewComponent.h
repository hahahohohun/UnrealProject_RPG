#pragma once


#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC/PC_Enum.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC_ArcSplinePreviewComponent.generated.h"

class USplineComponent;
class USplineMeshComponent;

USTRUCT(BlueprintType)
struct FPC_ArcPreviewConfig
{
	GENERATED_BODY()

	// 표시/시뮬레이션 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AimUpdateHz = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSimTime = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SimFrequency = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileRadius = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSplinePoints = 64;

	// 중력 오버라이드(0이면 월드값 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverrideGravityZ = 0.f;

	// 시각화(스플라인 메시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SplineMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SplineMeshPoolSize = 32;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PC_API UPC_ArcSplinePreviewComponent : public UActorComponent
{
	GENERATED_BODY()
	UPC_ArcSplinePreviewComponent();

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="ArcPreview")
	void BeginPreview();

	UFUNCTION(BlueprintCallable, Category="ArcPreview")
	void EndPreview();

	// (매 프레임 or 일정주기) 시작점/속도 기반으로 Predict → 스플라인 갱신
	// 성공 시 true
	UFUNCTION(BlueprintCallable, Category="ArcPreview")
	bool UpdateFromStartVelocity(const FVector& Start, const FVector& LaunchVelocity);

	UFUNCTION(BlueprintCallable, Category="ArcPreview")
	bool UpdateFromPredictParams(const struct FPredictProjectilePathParams& InParams);

	// 마지막 경로 포인트(발사 시 참고용)
	const TArray<FVector>& GetLastPathPoints() const { return LastPathPoints; }

	//Begin할때만 지워짐
	const FVector GetLastPathPoint() const {return LastPoint;}
	
	// 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ArcPreview")
	FPC_ArcPreviewConfig Config;
	
	// 내부 스플라인 가져오기(필요시)
	USplineComponent* GetSpline() const { return ArcSpline; }

	// 무시할 액터 추가(자기 자신, 소유자 등)
	void AddActorToIgnore(AActor* Actor);

	bool IsActivePreview() const { return bActive; }

private:

	void BuildSplineFromPoints(const TArray<FVector>& Points);
	void ClearSpline();
	void HideAllSegments();
	void EnsurePool();

private:
	UPROPERTY(Transient)
	USplineComponent* ArcSpline = nullptr;

	UPROPERTY(Transient)
	TArray<USplineMeshComponent*> SplineMeshPool;

	UPROPERTY(Transient)
	TArray<FVector> LastPathPoints;

	UPROPERTY(Transient)
	FVector LastPoint;
	
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> ActorsToIgnore;

	bool bActive = false;
	float Acc = 0.f;
};
