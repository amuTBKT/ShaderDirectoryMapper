// Copyright 2024 Amit Kumar Mehar. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ShaderDirectoryMapper : ModuleRules
{
	public ShaderDirectoryMapper(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCHUsage = PCHUsageMode.NoPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"Projects",
				"RenderCore",
				"CoreUObject",
				"DeveloperSettings",
            }
		);
    }
}
