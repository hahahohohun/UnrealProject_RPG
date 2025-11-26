#pragma once


#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PC/PC_Enum.h"
#include "PC_AimComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_AimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Called when the game starts
	UPC_AimComponent();
	
protected:
	virtual void BeginPlay() override;
	void CalcAimOffset(float DeltaTime);
	bool bCameraBlending = false;
	//
	UPROPERTY(BlueprintReadOnly)
	FRotator AimOffsetRotation = FRotator::ZeroRotator;
	bool bCameraAnimPlaying = false;
	EPC_CameraType PrevCameraType = EPC_CameraType::Normal;
	FTimerHandle CameraAnimTimerHandle;
	UFUNCTION()
	void OnCameraAnimFinished();
	//

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	EPC_CameraType CurrentCameraType = EPC_CameraType::Normal;
	void SwitchCamera(EPC_CameraType CameraType);
	void PlayCameraAnim(EPC_CameraType CameraType, float Time);

	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr;
};

