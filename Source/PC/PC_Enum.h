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
	CrowdControl,
	RootMotionDistance,
	HitPart,
	StatusEffect,
	Num,
};

UENUM(BlueprintType)
enum class EPC_EnemyStateType : uint8
{
	None = 0,
	Patrol = 1,
	Battle = 2,
	Investigating = 3,
	CrowdControlled = 4,
	Dead = 5,
	SKillUsing,
	ReactAttackBreak,
	Num
};

UENUM(BlueprintType)
enum class EPC_DeadType : uint8
{
	None = 0,
	Normal = 1,
	Num
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
	Assassinate = 7,
	Num,
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
	Assassinate = 7,
	Num,
};

UENUM(BlueprintType)
enum class EPC_SkillObjectType :uint8
{
	None,
	Projectile,
	Num
};

UENUM(BlueprintType)
enum class EPC_StatusEffectType : uint8
{
	None = 0,
	Heal = 1,
	AttackPowerUp = 2,
	MoveSpeed = 3,
	Stamina = 4,
	Num,
};

UENUM(BlueprintType)
enum class EPC_StatusEffectApplyType : uint8
{
	None            UMETA(DisplayName = "None"),
	Instant         UMETA(DisplayName = "즉발형"),
	OverTime        UMETA(DisplayName = "지속형 (초당 Tick 적용)"),
	StatModifier    UMETA(DisplayName = "버프형 (스탯 변경 후 원복)"),
};

// 값 해석 방법: +인지 ×인지
UENUM(BlueprintType)
enum class EPC_ValueMode : uint8
{
	None = 0,
	Additive,
	Multiplicative,
	Num,
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
	TargetPlayer,
	Num,
};

UENUM(BlueprintType)
enum class EPC_ExecType : uint8
{
	None,
	Dash,
	DashToTarget,
	Projectile,
	FireMultipleProjectile,
	FireCircularRain,
	Dot,
	Pushback,
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
	Num,
};

UENUM(BlueprintType)
enum class EPC_ExecCollisionType :uint8
{
	None,
	Box,
	Sphere,
	Capsule,
	Num,
};

UENUM(BlueprintType)
enum class EPC_CrowdControlType : uint8
{
	None,
	Pushback,
	Stun,
	Freeze,
	Num
};

UENUM(BlueprintType)
enum class EPC_MovementType : uint8
{
	None,
	Walk,
	Strafe,
	Run,
	Num,
};

UENUM(BlueprintType)
enum class EPC_HitPartType : uint8
{
	None,
	Body,
	Arm_l,
	Arm_r,
	Leg_l,
	Leg_r,
	Head,
	Num
};

UENUM(BlueprintType)
enum class EPC_CameraShakeMagnitudeType : uint8
{
	None,
	Weak,
	Normal,
	Strong,
	Assassinate,
	Num
};
