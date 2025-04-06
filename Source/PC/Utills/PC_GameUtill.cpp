#include "PC_GameUtill.h"

#include "Kismet/GameplayStatics.h"
#include "PC/PC.h"
#include "PC/Subsystem/PC_DataSubsystem.h"

FPC_CharacterStatTableRow* FPC_GameUtil::GetCharacterStatData(EPC_CharacterType CharacterType)
{
	if (GEngine)
	{
		if(UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GEngine->GetCurrentPlayWorld()))
		{
			if (UPC_DataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UPC_DataSubsystem>())
			{
				if (const UDataTable* DataTable = DataSubsystem->GetTable(EPC_DataTableType::CharacterStat))
				{
					TArray<FPC_CharacterStatTableRow*> CharacterTableRows;
					DataTable->GetAllRows(TEXT(""), CharacterTableRows);

					if (FPC_CharacterStatTableRow** FoundRow = CharacterTableRows.FindByPredicate([CharacterType](const FPC_CharacterStatTableRow* Row)
					{
						return Row->CharacterType == CharacterType;
					}))
					{
						return *FoundRow;
					}
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("CharacterStatData is Invalid"));
	return nullptr;
}
