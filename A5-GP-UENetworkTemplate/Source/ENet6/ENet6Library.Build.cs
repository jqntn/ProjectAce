using System;
using System.IO;
using UnrealBuildTool;

public class ENet6Library : ModuleRules
{
	public ENet6Library(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        if (Target.Platform == UnrealTargetPlatform.Win64)
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "x86_64", "enet6.lib"));
        else if (Target.Platform == UnrealTargetPlatform.Linux)
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "x86_64", "libenet6.a"));
    }
}

