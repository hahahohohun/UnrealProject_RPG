// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "PC_SkillObject.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "PC/PC_Enum.h"
#include "PC_SentinelProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;

UENUM()
enum class EPC_SentinelPhase : uint8
{
	Orbit,
	Launched,
	Finished
};

USTRUCT(BlueprintType)
struct FPC_SentinelParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float OrbitRadius    = 160.f; //플레이어를 중심으로 회전할때의 반경 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float OrbitHeight    = 60.f; //궤도의 높이
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AngularSpeed   = 180.f;   // 회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TriggerRange   = 1200.f;  // 감지 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DamageRadius   = 80.f;    // 근접 판정 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float IdleLifeTime   = 8.f;     // 대기 유지 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelayAttackTime = 2.f;     // 발사 후 생존
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float InitialSpeed   = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxSpeed       = 2800.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HomingAccel    = 9000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool  bHoming        = true; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float StartAngleDeg  = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage  = 0.f;
	
};


UCLASS()
class PC_API APC_SentinelProjectile : public APC_SkillObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APC_SentinelProjectile();
	
	// 스폰 직후 SkillComponent에서 1회 설정
	void InitSentinel(const FPC_SentinelParams& InParams);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* DetectSphere = nullptr;

	FPC_SentinelParams Cfg;
	EPC_SentinelPhase Phase = EPC_SentinelPhase::Orbit;
	
	float ThetaDeg = 0.f;
	float IdleTimeLeft = 0.f;
	float DelayAttackTime = 0.f;
	// 타깃/후보
	TWeakObjectPtr<AActor> Target;
	TSet<TWeakObjectPtr<AActor>> Candidates;

	// --- 갱신/로직 ---
	void UpdateOrbit(float deltaTime);
	void ExecutePickAndLaunch();
	AActor* FindBestTarget() const;
	bool IsEnemyValid(AActor* Other) const;

	void EnterLaunch(AActor* InTarget);
	void FinishSelf(); // Finished 처리(기존 ProcessDestroy 호출)
	
	// --- Overlap 이벤트 ---
	UFUNCTION()
	void OnDetectBegin(UPrimitiveComponent* Overlapped, AActor* OtherActor,
					   UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnDetectEnd(UPrimitiveComponent* Overlapped, AActor* OtherActor,
					 UPrimitiveComponent* OtherComp, int32 BodyIndex);

	UFUNCTION()
	void OnProximityBegin(UPrimitiveComponent* Overlapped, AActor* OtherActor,
						  UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);
};