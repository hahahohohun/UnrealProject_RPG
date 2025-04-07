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
	Enemy,
	Num
};
