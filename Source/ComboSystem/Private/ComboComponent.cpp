#include "ComboComponent.h"	
#include "Components/SkeletalMeshComponent.h"

UComboComponent::UComboComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


void UComboComponent::OnAttackAnimationEnded(UAnimMontage *_montage, bool _wasInterrupted)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	// Debugging messages
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(5, 1.5f, FColor::Red, TEXT("Attack animation ended"));
	}
#endif
}

// Called when the game starts
void UComboComponent::BeginPlay()
{
	Super::BeginPlay();

	if (moveSet == nullptr)
	{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		// Debugging messages
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, TEXT("Moveset table not selected"));
		}
#endif
		//do not continue below into initialization if the moveset table is not selected
		return;
	}

	// Init moveset graph
	moveSetGraph = *(new ComboGraph());
	moveSetGraph.CreateComboGraph(moveSet);
}


// Called every frame
void UComboComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Increase variable for time elapsed since the last input was received
	timeSinceLastAttackInput += DeltaTime;

	// Check if the attack cooldown is currently active
	// Do this before checking if the combo sequence should reset otherwise we end up adding delta time to it on the same frame that the cooldown has been activated
	if (canAttack == false)
	{
		// No point increasing the attack recovery timer if we can already attack so we do it inside this 'if' block
		attackRecoveryCooldownTimer += DeltaTime;

		if (attackRecoveryCooldownTimer >= attackRecoveryCooldown)
		{
			canAttack = true;
			attackRecoveryCooldownTimer = 0.0f;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
			// Debugging messages
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Yellow, TEXT("Attack recovery cooldown timer has completed"));
			}
#endif
		}
	}

	// Check if current sequence should be reset because the player took too long to perform the next attack in the chain
	else if (timeSinceLastAttackInput >= timeBeforeComboReset)
	{
		//ResetComboSequence();
	}
}

void UComboComponent::AttackInput(AttackType_Enum attackType)
{

	if (moveSet == nullptr)
	{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		// Debugging messages
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(0, 1.5f, FColor::Red, TEXT("Moveset table not selected"));
		}
#endif

		//Do not attempt attack without moveset table
		return;

	}


#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	// Debugging messages
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(9, 1.5f, FColor::Yellow, TEXT("Attempting Attack: " + attackType));
	}
#endif

	// Do nothing if the player cannot attack right now
	if (canAttack == false)
	{
		return;
	}


	// Evaluate current combo sequence with respect to combo graph to figure out which move to perform
	currentComboSequence.Add(attackType);

	FComboGraphNode* attackToPerform = nullptr;
	if (lastPerformedAttack == nullptr)
	{
		attackToPerform = moveSetGraph.FastFindNodeWithAttackChain(currentComboSequence);
	}
	else
	{
		attackToPerform = moveSetGraph.FastFindNodeWithLastPerformedAttack(currentComboSequence, lastPerformedAttack);
	}

	// Check if move is not found
	if (attackToPerform == nullptr)
	{
		// Reset the combo sequence because invalid move chain
		ResetComboSequence();

		// Don't process the rest
		return;
	}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	// Debugging messages
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.5f, FColor::Yellow, TEXT("Performing Attack: " + attackToPerform->attackData->moveName));
	}
#endif


	TArray<USkeletalMeshComponent*> Components;
	AActor* Actor = GetOwner();
	Actor->GetComponents<USkeletalMeshComponent>(Components);
	for (int32 i = 0; i < Components.Num(); i++)
	{
		USkeletalMeshComponent* skeletalMeshComponent = Components[i];
		UAnimInstance* SkeletalMeshAnimInstance = skeletalMeshComponent->GetAnimInstance();		
		SkeletalMeshAnimInstance->Montage_Play(attackToPerform->attackData->attackAnimation);


		FOnMontageEnded blendOutDel;
		blendOutDel.BindUObject(this, &UComboComponent::OnAttackAnimationEnded);
		SkeletalMeshAnimInstance->Montage_SetBlendingOutDelegate(blendOutDel);
		
		//SkeletalMeshAnimInstance->OnMontageBlendingOut.Add(OnAttackAnimationEnded);
	}

	//check if that was the last possible attack in its chain and activate the recovery cooldown if it is
	if (attackToPerform->childrenNodes.Num() < 1)
	{
		ResetComboSequence();
	}
}

void UComboComponent::ResetComboSequence()
{
	// Clear the current combo sequence
	currentComboSequence.Empty();

	// Activate cooldown for attacks
	ActivateAttackCooldown();

	OnComboBrokenDelegate.Broadcast();

	lastPerformedAttack = nullptr;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	// Debugging messages
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.5f, FColor::Yellow, TEXT("Combo Sequence has been reset"));
	}
#endif
}

void UComboComponent::ActivateAttackCooldown()
{
	canAttack = false;
	attackRecoveryCooldownTimer = 0.0f;
}