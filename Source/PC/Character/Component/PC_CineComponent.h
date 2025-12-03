#pragma once


#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "Components/ActorComponent.h"
#include "PC/PC_Enum.h"
#include "PC_CineComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PC_API UPC_CineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Called when the game starts
	UPC_CineComponent();
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSequenceFinished();

	ACineCameraActor* SetupPossessableCameraByTag();

	void PlaySequenceOnActor(AActor* CenterActor, ULevelSequence* Asset);
	TWeakObjectPtr<ACharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	ULevelSequence* LevelSequence;

	UPROPERTY()
	ULevelSequencePlayer* ActiveLevelSequencePlayer;

	UPROPERTY()
	ALevelSequenceActor* ActiveSequenceActor;

	UPROPERTY()
	ACineCameraActor* SequenceCamera;

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY(EditDefaultsOnly, Category="Cine")
	float BlendInTime = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category="Cine")
	float BlendOutTime = 0.25f;
	
	
	
};

