// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboSystemGameMode.h"
#include "ComboSystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

AComboSystemGameMode::AComboSystemGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
