// Copyright SpiderHero Team. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SpiderHeroTarget : TargetRules
{
	public SpiderHeroTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("SpiderHero");
	}
}
