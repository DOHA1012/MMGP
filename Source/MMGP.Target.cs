// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MMGPTarget : TargetRules
{
	public MMGPTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("MMGP");
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            bWarningsAsErrors = false;
        }
    }
}
