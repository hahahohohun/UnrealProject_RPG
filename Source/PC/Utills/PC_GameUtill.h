#pragma once
#include "Kismet/GameplayStatics.h"
#include "PC/PC_Enum.h"
#include "PC/Data/PC_TableRows.h"
#include "PC/Subsystem/PC_DataSubsystem.h"

class FPC_GameUtil
{
public:
	static FPC_CharacterStatTableRow* GetCharacterStatData(EPC_CharacterType CharacterType);
	static FPC_EnemyTableRow* GetEnemyData(EPC_CharacterType EnemyType);

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

