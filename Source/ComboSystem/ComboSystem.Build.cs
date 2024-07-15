// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ComboSystem : ModuleRules
{
	public ComboSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
	}
}
