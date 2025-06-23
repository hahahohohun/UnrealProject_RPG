#include "PC_GameUtill.h"
#include "PC/PC.h"

FPC_CharacterStatTableRow* FPC_GameUtil::GetCharacterStatData(EPC_CharacterType CharacterType)
{
	TArray<FPC_CharacterStatTableRow*> CharacterTableRows = GetAllRows<FPC_CharacterStatTableRow>(EPC_DataTableType::CharacterStat);
	if (FPC_CharacterStatTableRow** FoundRow = CharacterTableRows.FindByPredicate([CharacterType](const FPC_CharacterStatTableRow* Row)
	{
		return Row->CharacterType == CharacterType;
	}))
	{
		return *FoundRow;
	}
	
	UE_LOG(LogPC, Error, TEXT("CharacterStatData is Invalid"));
	return nullptr;
}

FPC_EnemyTableRow* FPC_GameUtil::GetEnemyData(EPC_CharacterType CharacterType)
{
	TArray<FPC_EnemyTableRow*> CharacterTableRows = GetAllRows<FPC_EnemyTableRow>(EPC_DataTableType::Enemy);
	if (FPC_EnemyTableRow** FoundRow = CharacterTableRows.FindByPredicate([CharacterType](const FPC_EnemyTableRow* Row)
	{
		return Row->EnemyType == CharacterType;
	}))
	{
		return *FoundRow;
	}
	
	UE_LOG(LogPC, Error, TEXT("CharacterStatData is Invalid"));
	return nullptr;
}

FPC_WeaponTableRow* FPC_GameUtil::GetWeaponData(uint8 WeaponId)
{
	TArray<FPC_WeaponTableRow*> EnemyTableRows = GetAllRows<FPC_WeaponTableRow>(EPC_DataTableType::Weapon);
	if (FPC_WeaponTableRow** FoundRow = EnemyTableRows.FindByPredicate([WeaponId](const FPC_WeaponTableRow* Row)
	{
		return Row->WeaponId == WeaponId;
	}))
	{
		return *FoundRow;
	}
	
	UE_LOG(LogPC, Error, TEXT("WeaponData is Invalid"));
	return nullptr;
}

UPC_CameraDataAsset* FPC_GameUtil::GetCameraData(EPC_CameraType CameraType)
{
	if (GEngine)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_DataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UPC_DataSubsystem>())
			{
				return *DataSubsystem->CameraData.Find(CameraType);
			}
		}
	}

	return nullptr;
}

FPC_SkillTableRow* FPC_GameUtil::GetSkillData(uint32 Skillid)
{
	TArray<FPC_SkillTableRow*> EnemyTableRows = GetAllRows<FPC_SkillTableRow>(EPC_DataTableType::Skill);
	if (FPC_SkillTableRow** FoundRow = EnemyTableRows.FindByPredicate([Skillid](const FPC_SkillTableRow* Row)
	{
		return Row->DataId == Skillid;
	}))
	{
		return *FoundRow;
	}
	
	UE_LOG(LogPC, Error, TEXT("Skill data is Invalid"));
	return nullptr;
}

FPC_SkillObjectTableRow* FPC_GameUtil::GetSkillObjectData(uint32 Skillid)
{
	TArray<FPC_SkillObjectTableRow*> EnemyTableRows = GetAllRows<FPC_SkillObjectTableRow>(EPC_DataTableType::SkillObject);
	if (FPC_SkillObjectTableRow** FoundRow = EnemyTableRows.FindByPredicate([Skillid](const FPC_SkillObjectTableRow* Row)
	{
		return Row->DataId == Skillid;
	}))
	{
		return *FoundRow;
	}
	
	UE_LOG(LogPC, Error, TEXT("skill object data is Invalid"));
	return nullptr;
}

FPC_ExecTableRow* FPC_GameUtil::GetExecData(uint32 Uint32)
{
	TArray<FPC_ExecTableRow*> EnemyTableRows = GetAllRows<FPC_ExecTableRow>(EPC_DataTableType::Exec);
	if (FPC_ExecTableRow** FoundRow = EnemyTableRows.FindByPredicate([Uint32](const FPC_ExecTableRow* Row)
	{
		return Row->DataId == Uint32;
	}))
	{
		return *FoundRow;
	}
	
	UE_LOG(LogPC, Error, TEXT("ExecData data is Invalid"));
	return nullptr;
}

void FPC_GameUtil::CameraShake()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GEngine->GetCurrentPlayWorld(), 0))
	{
		PlayerController->ClientStartCameraShake(UPC_LegacyCameraShake::StaticClass());
	}
}


