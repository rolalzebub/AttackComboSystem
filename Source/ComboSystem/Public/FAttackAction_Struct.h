#pragma once
#include "AttackType_Enum.h"
#include "Engine/DataTable.h"
#include "FAttackAction_Struct.generated.h"

/// <summary>
/// This struct outlines the members of a row in the combos data table.
/// </summary>
USTRUCT(BlueprintType)
struct COMBOSYSTEM_API FAttackAction_Struct : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
		
		/// <summary>
		/// The type of attack a row represents.
		/// </summary>
		/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<AttackType_Enum> attackType;*/

	/// <summary>
	/// The name for the attack this row represents.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString moveName;

	/// <summary>
	/// The attack sequence required to be input to activate this attack.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TEnumAsByte<AttackType_Enum>> requiredSequenceToActivateAttack;


	/// <summary>
	/// The animation to be used for this attack.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* attackAnimation;

};
