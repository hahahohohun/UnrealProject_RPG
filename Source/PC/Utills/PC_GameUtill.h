#pragma once
#include "Kismet/GameplayStatics.h"
#include "PC/PC_Enum.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Subsystem/PC_DataSubsystem.h"
#include "PC/Cometic/PC_LegacyCameraShake.h"

class UPC_CameraDataAsset;

class FPC_GameUtil
{
public:
	static FPC_CharacterStatTableRow* GetCharacterStatData(EPC_CharacterType CharacterType);
	static FPC_EnemyTableRow* GetEnemyData(EPC_CharacterType EnemyType);
	static FPC_WeaponTableRow* GetWeaponData(uint8 WeaponId);
	static UPC_CameraDataAsset* GetCameraData(EPC_CameraType CameraType);
	static FPC_SkillTableRow* GetSkillData(uint32 Uint32);
	static FPC_SkillObjectTableRow* GetSkillObjectData(uint32 Uint32);
	static FPC_ExecTableRow* GetExecData(uint32 Uint32);

	static void CameraShake();
	template <typename T>
	static TArray<T*> GetAllRows(EPC_DataTableType DataTableType);
};

template <typename T>
TArray<T*> FPC_GameUtil::GetAllRows(EPC_DataTableType CharacterType)
{
	if (GEngine)
	{
		if(UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_DataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UPC_DataSubsystem>())
			{
				if (const UDataTable* DataTable = DataSubsystem->GetTable(CharacterType))
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
