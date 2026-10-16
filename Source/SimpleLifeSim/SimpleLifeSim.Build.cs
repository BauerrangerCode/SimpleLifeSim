// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SimpleLifeSim : ModuleRules
{
	public SimpleLifeSim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"KeyCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"SimpleLifeSim",
			"SimpleLifeSim/Variant_Platforming",
			"SimpleLifeSim/Variant_Platforming/Animation",
			"SimpleLifeSim/Variant_Combat",
			"SimpleLifeSim/Variant_Combat/AI",
			"SimpleLifeSim/Variant_Combat/Animation",
			"SimpleLifeSim/Variant_Combat/Gameplay",
			"SimpleLifeSim/Variant_Combat/Interfaces",
			"SimpleLifeSim/Variant_Combat/UI",
			"SimpleLifeSim/Variant_SideScrolling",
			"SimpleLifeSim/Variant_SideScrolling/AI",
			"SimpleLifeSim/Variant_SideScrolling/Gameplay",
			"SimpleLifeSim/Variant_SideScrolling/Interfaces",
			"SimpleLifeSim/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
