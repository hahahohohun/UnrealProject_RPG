#pragma once
#include "Kismet/GameplayStatics.h"
#include "PC/PC_Enum.h"
#include "PC/Character/Component/PC_StatComponent.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/Data/PC_TableRows.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "PC/Subsystem/PC_DataSubsystem.h"
namespace PC_EnemyRange
{
	constexpr float Under   = 300.f;
	constexpr float NearRange   = 500.f;
	constexpr float MiddleRange = 1000.f;
}

class UPC_CameraDataAsset;

class FPC_GameUtil
{
public:
	static FPC_CharacterStatTableRow* GetCharacterStatData(uint32 CharacterType);
	static FPC_EnemyTableRow* GetEnemyData(uint32 EnemyType);
	
	static FPC_WeaponTableRow* GetWeaponData(uint32 WeaponId);
	static UPC_CameraDataAsset* GetCameraData(EPC_CameraType CameraType);
	static UPC_GameDataAsset* GetGameData();
	
	static FPC_SkillTableRow* GetSkillData(uint32 skillId);
	static FPC_SkillObjectTableRow* GetSkillObjectData(uint32 skillObjectId);
	static FPC_ExecTableRow* GetExecData(uint32 execId);
	static FPC_CrowdControlTableRow* GetCrowdControlData(uint32 crowdId);
	static FPC_StatusEffectTableRow* GetStatusEffectData(uint32 statusEffectId);
	
	static float GetRootMotionDistanceData(FSoftObjectPath& ObjectPath);
	static float CalculateRootMotionDistance(UAnimMontage* AnimMontage);

	static FPC_HitPartListRow* GetHitPartData(FSoftObjectPath& ObjectPath);
	static EPC_HitPartType GetHitPartTypeByName(FName BoneName, UDataAsset* DataAsset);
	static UAnimMontage* GetProperAttackMontage(TArray<TObjectPtr<UAnimMontage>>& AlreadyPlayedMontage, AActor* AttackActor, FVector TargetPos);
	
	static AActor* GetBestTargetByViewAngle(APlayerController* PlayerController, TArray<AActor*> TargetActors, bool ShouldGetNotInBattleActor, float MaxAngle);
	static ECollisionChannel GetAttackCollisionChannel(uint32 Dataid);
	
	static uint32 GetSkillId(UPC_PlayerDataAsset* PlayerDataAsset, EPC_SkillSlotType SlotType, EPC_CharacterStanceType CharacterStance
	,bool bInSpecialAttack);

	//전투효과
	static void CameraShake(EPC_CameraShakeMagnitudeType Type);
	static void PlayStopDilation(const UObject* WorldObject, float Duration, float Dilation);
	static void PlayHitMaterial(ACharacter* DamageCharacter);
	//
	static void PlaySFXAtLocation(UObject* WorldContextObject, USoundBase* SFX, const FVector& Location);

	//todo
	//static void ApplyHitReactionKnockback(const UObject* WorldObject, float Amount);
	
	static FPC_CharacterStatModifier MakeCharacterStatModifierFromRow(const FPC_StatusEffectTableRow& Row, const FPC_CharacterStatTableRow& BaseStat);
	
	static void SpawnEffectAtLocation(UObject* WorldContextObj, UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation, float Scale = 1);
	static void SpawnEffectAtLocation(UObject* WorldContextObj, UParticleSystem* ParticleSystem, FVector Location, FRotator Rotation, float Scale = 1);

	static UNiagaraComponent* SpawnEffectAttached(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent, FName AttachPointName, FVector Location, FRotator Rotation, EAttachLocation::Type LocationType, bool bAutoDestroy);
	static UParticleSystemComponent* SpawnEffectAttached(UParticleSystem* ParticleSystem, USceneComponent* AttachToComponent, FName AttachPointName, FVector Location, FRotator Rotation, EAttachLocation::Type LocationType, bool bAutoDestroy);

	static void SpawnDamageFloater(ACharacter* DamageCharacter, int32 Damge);
	
	static FVector FindSurfacePos(ACharacter* Character, FVector& CurrentPos);
	
	//디버그용
	static bool IsDebugDrawing(UObject* WorldContextObject);
	static void AddOnScreenDebugMessage(FString msg);
	//
	template <typename T>
	static TArray<T*> GetAllRows(EPC_DataTableType DataTableType);

	static EPC_ProximityType GetTargetProximity(AActor* TargetActor, AActor* CurrentActor,float Under, float Near, float Middle, FVector CurrentActorOffset);
	static FColor GetHitPartColor(EPC_HitPartType PartType);
	static FColor GetHitPartColor(FPC_HitPartListRow* ListRow, FName BoneName);
	static float GetHitPartAddDamage(FPC_HitPartListRow* ListRow, FName BoneName);
	//static float GetCalcTotalHitPartDamage(float Damage, FPC_HitPartListRow* ListRow, FName BoneName);
	static float GetCalcTotalNormalDamage(float Damage, AActor* HitActor, FName BoneName);
	static FTransform GetSocketTransform(AActor* Actor, FName BoneName); //소켓찾고 없으면 무기소켓도
private:

};

template <typename T>
TArray<T*> FPC_GameUtil::GetAllRows(EPC_DataTableType Type)
{
	if (GEngine)
	{
		if(UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_DataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UPC_DataSubsystem>())
			{
				if (const UDataTable* DataTable = DataSubsystem->GetTable(Type))
				{
					TArray<T*> TableRows;
					DataTable->GetAllRows(TEXT(""), TableRows);

					return TableRows;
				}
			}
		}
	}

	return TArray<T*>();
}
