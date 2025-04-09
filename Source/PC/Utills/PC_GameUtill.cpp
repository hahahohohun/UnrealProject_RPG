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
