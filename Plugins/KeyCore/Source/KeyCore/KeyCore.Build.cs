// Copyright Torben Borghoff, Keygen-Games

using UnrealBuildTool;

public class KeyCore : ModuleRules
{
	public KeyCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"FieldNotification",
			"ModelViewViewModel",
			"ModularGameplay"
		});
	}
}
