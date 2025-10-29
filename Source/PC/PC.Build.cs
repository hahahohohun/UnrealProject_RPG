// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PC : ModuleRules
{
	public PC(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "Slate", "SlateCore","InputCore", "HeadMountedDisplay", "EnhancedInput",
			"UMG", "Niagara" , "AIModule", "GameplayTasks", "GameplayCameras" ,"Niagara", "NavigationSystem"
			, "AssetRegistry","EngineCameras", "SkinnedDecalComponent"
		});
	}
}
