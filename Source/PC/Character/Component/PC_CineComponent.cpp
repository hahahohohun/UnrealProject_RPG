#include "PC_CineComponent.h"

#include "EngineUtils.h"
#include "LevelSequenceActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UPC_CineComponent::UPC_CineComponent()
{
	//PrimaryComponentTick.bCanEverTick = true;
}

void UPC_CineComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UPC_CineComponent::PlaySequenceOnActor(AActor* CenterActor, ULevelSequence* Asset)
{
	if (!Asset || !CenterActor)
		return;
		
	LevelSequence = Asset;
	TargetActor = CenterActor;

	UWorld* World = GetWorld();
	if (!World)
		return;

	if(ActiveLevelSequencePlayer)
		return;

	SequenceCamera = SetupPossessableCameraByTag();
	if (!SequenceCamera)
		return;

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bAutoPlay = false;

	ActiveLevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer
	(
		World,
		LevelSequence,
		Settings,
		ActiveSequenceActor
		);

	if(!ActiveLevelSequencePlayer || !ActiveSequenceActor)
		return;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if(!PlayerController)
		return;

	ActiveLevelSequencePlayer->OnFinished.RemoveAll(this);
	ActiveLevelSequencePlayer->OnFinished.AddDynamic(this, &UPC_CineComponent::OnSequenceFinished);

	//카메라전환
	PlayerController->SetViewTargetWithBlend(
		SequenceCamera,
		BlendInTime,
		VTBlend_EaseInOut,
		2.0f);

	ActiveLevelSequencePlayer->Play();
	
}


void UPC_CineComponent::OnSequenceFinished()
{
	UWorld* World = GetWorld();
	if(!World)
		return;
	
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if(!PlayerController)
		return;

	//끝났으면 다시 플레이어로 카메라 돌려주기
	if(PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(
			PlayerController->GetPawn(),
			BlendOutTime,
			VTBlend_EaseInOut,
			2.0f,
			true);
	}

	if(ActiveSequenceActor)
	{
		ActiveSequenceActor->Destroy();
		ActiveSequenceActor = nullptr;
	}

	ActiveLevelSequencePlayer = nullptr;

	SequenceCamera->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

ACineCameraActor* UPC_CineComponent::SetupPossessableCameraByTag()
{
	if(!GetWorld() || !TargetActor)
	{
		return nullptr;
	}

	USceneComponent* TargetRoot = TargetActor->GetRootComponent();
	if(!TargetRoot)
	{
		return nullptr;
	}

	const FName CameraTag = TEXT("SequenceSkillCamera");
	ACineCameraActor* CineCameraActor = nullptr;

	for(TActorIterator<ACineCameraActor> It(GetWorld()); It; ++It)
	{
		ACineCameraActor* CurrentCam = *It;

		if(CurrentCam && CurrentCam->Tags.Contains(CameraTag))
		{
			CineCameraActor = CurrentCam;
			break;
		}
	}

	if(!CineCameraActor)
	{
		UE_LOG(LogTemp, Error, TEXT("SequenceSkillCamera not find"));
		return nullptr;
	}

	//타겟한테 카메라를 어태치
	CineCameraActor->AttachToComponent(TargetRoot,
		FAttachmentTransformRules::KeepWorldTransform,
		NAME_None);

	return CineCameraActor;
}
