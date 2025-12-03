#pragma once
#include "Components/PoseableMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PC_AfterImageActor.generated.h"
UCLASS()
class PC_API APC_AfterImageActor : public AActor
{
	GENERATED_BODY()

public:
	APC_AfterImageActor();

	void InitFromMesh(USkeletalMeshComponent* SourceMesh);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AfterImage")
	UPoseableMeshComponent* PoseMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AfterImage")
	UMaterialInterface* AfterImageMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AfterImage")
	float Duration;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	float ElapsedTime;
};