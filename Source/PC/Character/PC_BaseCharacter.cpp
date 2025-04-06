// Copyright Epic Games, Inc. All Rights Reserved.

#include "PC_BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Component/PC_StatComponent.h"

//////////////////////////////////////////////////////////////////////////
// APCCharacter

APC_BaseCharacter::APC_BaseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	BattleComponent = CreateDefaultSubobject<UPC_BattleComponent>(TEXT("BattleComponent"));
	StatComponent = CreateDefaultSubobject<UPC_StatComponent>(TEXT("StatComponent"));
	
}

void APC_BaseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void APC_BaseCharacter::AttackTrace(bool bStart, FName TraceBoneName)
{
	if(BattleComponent)
	{
		if(bStart)
		{
			BattleComponent->StartTrace(TraceBoneName);
		}
		else
		{
			BattleComponent->EndTrace();
		}
	}
}

//



