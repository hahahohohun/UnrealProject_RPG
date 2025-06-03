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
void FPC_GameUtil::CameraShake()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GEngine->GetCurrentPlayWorld(), 0))
	{
		PlayerController->ClientStartCameraShake(UPC_LegacyCameraShake::StaticClass());
	}
}
