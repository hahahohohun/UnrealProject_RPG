// PC_CutsceneSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MovieSceneSequencePlayer.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "PC_CutsceneSubsystem.generated.h"

UCLASS()
class PC_API UPC_CutsceneSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPC_CutsceneSubsystem();

    virtual void Deinitialize() override;

    void StopCurrentCutscene(bool bResetPlayer = true);
    ULevelSequencePlayer* PlayCutscene(ULevelSequence* SequenceAsset, const FSimpleDelegate& InOnFinished);
    ULevelSequencePlayer* PlayCutsceneWithSettings(ULevelSequence* SequenceAsset, const FMovieSceneSequencePlaybackSettings& InSettings, const FSimpleDelegate& InOnFinished);
    ULevelSequencePlayer* GetCurrentPlayer() const { return CurrentPlayer; }

private:
    UPROPERTY()
    ULevelSequencePlayer* CurrentPlayer;

    UPROPERTY()
    ALevelSequenceActor* CurrentSequenceActor;
    
    FSimpleDelegate OnFinishedNativeDelegate;
    
    UFUNCTION()
    void OnSequenceFinishedInternal();
};