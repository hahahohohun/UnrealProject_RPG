// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimMontage.h"
#include "CoreMinimal.h"
#include "PC_BaseCharacter.h"
#include "PC_PlayableCharaceter.generated.h"


UCLASS()
class PC_API APC_PlayableCharaceter : public APC_BaseCharacter
{
	GENERATED_BODY()
	

public:
	// Sets default values for this character's properties
	APC_PlayableCharaceter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	virtual void Attack(const FInputActionValue& Value);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AttackAction;


	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsAttacking = false;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsSaveAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int AttackCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Anim)
	TArray<UAnimMontage*> AttackMontages;

	UFUNCTION(BlueprintCallable)
	void ResetComb();

	UFUNCTION(BlueprintCallable)
	void CombAttackSave();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
