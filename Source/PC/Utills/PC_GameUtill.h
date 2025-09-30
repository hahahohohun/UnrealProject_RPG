#pragma once
#include "Kismet/GameplayStatics.h"
#include "PC/PC_Enum.h"
#include "PC/Data/PC_PlayerDataAsset.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Subsystem/PC_DataSubsystem.h"
//#include "PC/Cometic/PC_LegacyCameraShake.h"
//#include "PC/Data/PC_PlayerDataAsset.h"

class UPC_CameraDataAsset;

class FPC_GameUtil
{
public:
	static FPC_CharacterStatTableRow* GetCharacterStatData(uint32 CharacterType);
	static FPC_EnemyTableRow* GetEnemyData(uint32 EnemyType);
	
	static FPC_WeaponTableRow* GetWeaponData(uint32 WeaponId);
	static UPC_CameraDataAsset* GetCameraData(EPC_CameraType CameraType);
	static FPC_SkillTableRow* GetSkillData(uint32 skillId);
	static FPC_SkillObjectTableRow* GetSkillObjectData(uint32 skillObjectId);
	static FPC_ExecTableRow* GetExecData(uint32 execId);
	static FPC_CrowdControlTableRow* GetCrowdControlData(uint32 crowdId);
	static float GetRootMotionDistanceData(FSoftObjectPath& ObjectPath);
	static float CalculateRootMotionDistance(UAnimMontage* AnimMontage);

	static FPC_HitPartListRow* GetHitPartData(FSoftObjectPath& ObjectPath);
	static EPC_HitPartType GetHitPartTypeByName(FName BoneName, UDataAsset* DataAsset);
	
	
	static UAnimMontage* GetProperAttackMontage(TArray<TObjectPtr<UAnimMontage>>& AnimMontages, TArray<TObjectPtr<UAnimMontage>>& AlreadyPlayedMontage,
		AActor* AttackActor, FVector TargetPos);
	static ECollisionChannel GetAttackCollisionChannel(uint32 Dataid);
	
	static uint32 GetSkillId(UPC_PlayerDataAsset* PlayerDataAsset, EPC_SkillSlotType SlotType, EPC_CharacterStanceType CharacterStance
	,bool bInSpecialAttack);
	
	static void CameraShake();
	static void SpawnEffectAtLocation(UObject* WorldContextObj, UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation);
	static void SpawnEffectAtLocation(UObject* WorldContextObj, UParticleSystem* ParticleSystem, FVector Location, FRotator Rotation);

	static UNiagaraComponent* SpawnEffectAttached(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent, FName AttachPointName, FVector Location, FRotator Rotation, EAttachLocation::Type LocationType, bool bAutoDestroy);
	static UParticleSystemComponent* SpawnEffectAttached(UParticleSystem* ParticleSystem, USceneComponent* AttachToComponent, FName AttachPointName, FVector Location, FRotator Rotation, EAttachLocation::Type LocationType, bool bAutoDestroy);

	static FVector FindSurfacePos(ACharacter* Character, FVector& CurrentPos);

	static bool IsDebugDrawing(UObject* WorldContextObject);
	
	template <typename T>
	static TArray<T*> GetAllRows(EPC_DataTableType DataTableType);

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
