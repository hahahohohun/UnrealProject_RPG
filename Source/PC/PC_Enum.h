#pragma once

UENUM()
enum class EPC_DataTableType : int32
{
	None,
	CharacterStat,
	Enemy,
	Num,
};

UENUM()
enum class EPC_CharacterType : int32
{
	None,
	Player,
	Kallari,
	Num
};

UENUM(BlueprintType)
enum class EPC_EnemyStateType : uint8
{
	None = 0,
	Patrol = 1,
	Battle = 2,
	Investigating =3,
	Num = 3,
};

UENUM(BlueprintType)
enum class EPC_AISenseType : uint8
{
	None,
	Sight,
	Hearing,
	Damage,
	Num,
};
