// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

using UnrealBuildTool;

public class AIFlowTests : ModuleRules
{
	public AIFlowTests(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"AIFlow",
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"AIModule",
			"CoreUObject",
			"Engine",
			"Flow",
			"GameplayTags",
		});
	}
}
