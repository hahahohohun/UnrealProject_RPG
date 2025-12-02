#include "PC_CutsceneSubsystem.h"

#include "LevelSequenceActor.h"
#include "Engine/World.h"

UPC_CutsceneSubsystem::UPC_CutsceneSubsystem()
    : CurrentPlayer(nullptr)
    , CurrentSequenceActor(nullptr)
{
}

void UPC_CutsceneSubsystem::Deinitialize()
{
    StopCurrentCutscene(true);
    Super::Deinitialize();
}

void UPC_CutsceneSubsystem::StopCurrentCutscene(bool bResetPlayer)
{
    if (CurrentPlayer)
    {
        // Dynamic delegate 해제
        CurrentPlayer->OnFinished.RemoveAll(this);

        if (bResetPlayer)
        {
            CurrentPlayer->Stop();
        }
    }

    if (CurrentSequenceActor && CurrentSequenceActor->IsValidLowLevel())
    {
        CurrentSequenceActor->Destroy();
    }

    CurrentPlayer        = nullptr;
    CurrentSequenceActor = nullptr;

    OnFinishedNativeDelegate.Unbind();
}

ULevelSequencePlayer* UPC_CutsceneSubsystem::PlayCutscene(
    ULevelSequence* SequenceAsset,
    const FSimpleDelegate& InOnFinished)
{
    FMovieSceneSequencePlaybackSettings Settings;
    Settings.bDisableMovementInput = true;
    Settings.bDisableLookAtInput   = true;
    Settings.bHidePlayer           = false;
    Settings.bHideHud              = true;
    Settings.bDisableCameraCuts    = false;
    Settings.FinishCompletionStateOverride = EMovieSceneCompletionModeOverride::ForceKeepState;
    return PlayCutsceneWithSettings(SequenceAsset, Settings, InOnFinished);
}

ULevelSequencePlayer* UPC_CutsceneSubsystem::PlayCutsceneWithSettings(
    ULevelSequence* SequenceAsset,
    const FMovieSceneSequencePlaybackSettings& InSettings,
    const FSimpleDelegate& InOnFinished)
{
    if (!SequenceAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("CutsceneSubsystem::PlayCutscene - SequenceAsset is null"));
        return nullptr;
    }

    if (UWorld* World = GetWorld())
    {
        // 기존 컷씬 정리
        StopCurrentCutscene(true);

        ALevelSequenceActor* SeqActor = nullptr;
        ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer(World, SequenceAsset, InSettings, SeqActor);

        if (!Player)
        {
            UE_LOG(LogTemp, Warning, TEXT("CutsceneSubsystem::PlayCutscene - Failed to create LevelSequencePlayer"));
            return nullptr;
        }

        CurrentPlayer        = Player;
        CurrentSequenceActor = SeqActor;
        OnFinishedNativeDelegate = InOnFinished;

        Player->OnFinished.RemoveAll(this);
        Player->OnFinished.AddDynamic(this, &UPC_CutsceneSubsystem::OnSequenceFinishedInternal);

        Player->Play();

        return Player;
    }
    return nullptr;
}

void UPC_CutsceneSubsystem::OnSequenceFinishedInternal()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            PC->SetViewTarget(Pawn);
        }
    }

    if (OnFinishedNativeDelegate.IsBound())
    {
        OnFinishedNativeDelegate.Execute();
    }
}
