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
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	void SwitchCamera(EPC_CameraType CameraType);
	void CalcAimOffset(float DeltaTime);
	
	UPROPERTY(BlueprintReadOnly)
	FRotator AimOffsetRotation = FRotator::ZeroRotator;

	EPC_CameraType CurrentCameraType = EPC_CameraType::Normal;
	bool bCameraBlending = false;
	
	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr;
};

