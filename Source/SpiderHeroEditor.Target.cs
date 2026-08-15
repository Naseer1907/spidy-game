// Copyright SpiderHero Team. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SpiderHeroEditorTarget : TargetRules
{
	public SpiderHeroEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("SpiderHero");
	}
}
