#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PC/PC_Enum.h"
#include "PC_TableRows.generated.h"

USTRUCT(BlueprintType)
struct FPC_CharacterStatTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	public:
	FPC_CharacterStatTableRow() :MaxHp(0.0f), Attack(0.0f), MovemnetSpeed(0.0f){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	EPC_CharacterType CharacterType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float MaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float MovemnetSpeed;

	FPC_CharacterStatTableRow operator+(const FPC_CharacterStatTableRow& Other) const
	{
		FPC_CharacterStatTableRow Result;

		Result.MaxHp = this->MaxHp + Other.MaxHp;
		Result.Attack = this->Attack + Other.Attack;
		Result.MovemnetSpeed = this->MovemnetSpeed + Other.MovemnetSpeed;
		
		return Result;
	}
	
};
