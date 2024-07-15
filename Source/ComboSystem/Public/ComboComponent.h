#pragma once

#include "Components/ActorComponent.h"
#include "ComboGraph.h"
#include "ComboComponent.generated.h"

/// <summary>
/// This class contains the logic for the combo component which takes inputs based on move type and performs attack animations for the given movesets.
/// The movesets are specified in a DataTable with rows based on ActionType_Struct.
/// </summary> 
UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent, BlueprintThreadSafe) )
class COMBOSYSTEM_API UComboComponent : public UActorComponent
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FComboBrokenDelegate);
	GENERATED_BODY()

// Public variables and properties
public:	


	UPROPERTY(BlueprintAssignable, Category = "Combat")
		FComboBrokenDelegate OnComboBrokenDelegate;

	/// <summary>
	/// Reference to the data table that contains the moveset for the character.
	/// This reference should be assigned in the editor.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* moveSet;

	/// <summary>
	/// How much time to allow before resetting the current combo sequence due to lack of attack inputs.
	/// Default value is 1 second.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float timeBeforeComboReset = 1.0f;

	/// <summary>
	/// Boolean indicating whether or not this actor is currently performing an attack.
	/// </summary>
	bool isAttacking = false;

	/// <summary>
	/// Cooldown after an attack chain ends before the actor can perform another attack.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float attackRecoveryCooldown = 2.0f;

	// Status of whether or not an actor can perform an attack this frame.
	bool canAttack = true;

// Private variables and properties
private:
	// Stores how much time has passed since the last attack input was received.
	float timeSinceLastAttackInput = 0.0f;


	// Stores how long it has been since the attack cooldown was activated
	float attackRecoveryCooldownTimer = 0.0f;

	// Array to store the current sequence of attack types that the actor is performing.
	TArray<TEnumAsByte<AttackType_Enum>> currentComboSequence;

	// The combo graph for the moveset specified in the moveset table
	ComboGraph moveSetGraph;

	FComboGraphNode* lastPerformedAttack = nullptr;

	void OnAttackAnimationEnded(UAnimMontage *_montage, bool _wasInterrupted);

	// Protected functions
protected:
	virtual void BeginPlay() override;

	// Public functions
public:	
	
	// Default constructor
	// Sets default values for this component's properties
	UComboComponent();

	// Tick function
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// <summary>
	/// This function should be called when the player presses an attack/action button.
	/// </summary>
	/// <param name="attackType:">The type of attack that is being performed.</param>
	UFUNCTION(BlueprintCallable, Category = Combat)
	void AttackInput(AttackType_Enum attackType);


	// Private functions
private:
	/// <summary>
	/// Resets the current combo sequence. Doing this activates the attack cooldown.
	/// </summary>
	void ResetComboSequence();

	void ActivateAttackCooldown();

};
