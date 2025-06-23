#pragma once

UENUM()
enum class EPC_DataTableType : int32
{
	None,
	CharacterStat,
	Enemy,
	Weapon,
	Skill,
	Exec,
	SkillObject,
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

UENUM(BlueprintType)
enum class EPC_ActionType : uint8
{
	None = 0,
	Move = 1,
	Jump = 2,
	Attack =3,
	Run = 4,
	Roll= 5,
	Guard = 6,
	Num = 7,
};

UENUM(BlueprintType)
enum class EPC_LockCauseType: uint8
{
	None = 0,
	Move = 1,
	Jump = 2,
	Attack = 3,
	Run = 4,
	Roll = 5,
	SpecialAction = 6,
	Num = 7,
};

UENUM(BlueprintType)
enum class EPC_SkillObjectType :uint8
{
	None,
	Projectile,
	Num
};

UENUM(BlueprintType)
enum class EPC_CharacterStanceType :uint8
{
	Sword = 0,
	Staff = 1,
};

UENUM(BlueprintType)
enum class EPC_CameraType : uint8
{
	None,
	Normal,
	Aim,
	Num
};


UENUM(BlueprintType)
enum class EPC_SkillTargetingType : uint8
{
	None,
	NoneTarget,
	ChainAttack,
	Multiple,
	Num,
};

UENUM(BlueprintType)
enum class EPC_ExecType : uint8
{
	None,
	Dash,
	Projectile,
	Num,
};

UENUM(BlueprintType)
enum class EPC_SkillSlotType : uint8
{
	None,
	Num_1,
	Num_2,
	Num_3,
	Num_4,
	Num_5,
	Num,
};
