// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "AIFlowActorBlackboardHelper.h"
#include "Templates/SubclassOf.h"

// Forward Declarations
class AActor;
class UBlackboardComponent;
class UBlackboardData;
class UFlowInjectComponentsManager;
class UAIFlowAsset;
class UFlowAsset;

/**
 * Static utility class containing shared logic for blackboard component resolution
 * used by various Flow nodes to reduce code duplication.
 */
class AIFLOW_API FFlowNodeBlackboardUtils
{
public:
	/**
	 * Resolves blackboard components for a single actor using common blackboard resolution logic.
	 *
	 * @param Actor The actor to find/add blackboard component on
	 * @param FlowAsset The flow asset to get default blackboard settings from
	 * @param InjectComponentsManager Optional component manager for injection (can be nullptr)
	 * @param SpecificBlackboardAsset Optional specific blackboard asset to use (overrides flow asset default)
	 * @param SearchRule Rule for searching for existing blackboard components
	 * @param InjectRule Rule for injecting missing blackboard components
	 * @return The resolved blackboard component, or nullptr if resolution failed
	 */
	static UBlackboardComponent* ResolveBlackboardComponentForActor(
		AActor* Actor,
		const UFlowAsset* FlowAsset,
		UFlowInjectComponentsManager* InjectComponentsManager,
		UBlackboardData* SpecificBlackboardAsset,
		EActorBlackboardSearchRule SearchRule,
		EActorBlackboardInjectRule InjectRule
	);

	/**
	 * Resolves blackboard components for multiple actors using common blackboard resolution logic.
	 *
	 * @param Actors The actors to find/add blackboard components on
	 * @param FlowAsset The flow asset to get default blackboard settings from
	 * @param InjectComponentsManager Optional component manager for injection (can be nullptr)
	 * @param SpecificBlackboardAsset Optional specific blackboard asset to use (overrides flow asset default)
	 * @param SearchRule Rule for searching for existing blackboard components
	 * @param InjectRule Rule for injecting missing blackboard components
	 * @return Array of resolved blackboard components (may contain nullptrs for failed resolutions)
	 */
	static TArray<UBlackboardComponent*> ResolveBlackboardComponentsForActors(
		const TArray<AActor*>& Actors,
		const UFlowAsset* FlowAsset,
		UFlowInjectComponentsManager* InjectComponentsManager,
		UBlackboardData* SpecificBlackboardAsset,
		EActorBlackboardSearchRule SearchRule,
		EActorBlackboardInjectRule InjectRule
	);

#if WITH_EDITOR
	/**
	 * Gets the blackboard asset to use for editor operations, with fallback logic.
	 *
	 * @param SpecificBlackboardAsset Optional specific blackboard asset
	 * @param FlowAsset The flow asset to get default blackboard from
	 * @return The blackboard asset to use for editor operations, or nullptr if none available
	 */
	static UBlackboardData* GetBlackboardAssetForEditor(
		UBlackboardData* SpecificBlackboardAsset,
		const UFlowAsset* FlowAsset
	);
#endif // WITH_EDITOR

private:
	/**
	 * Gets the blackboard component class and default blackboard asset from the flow asset.
	 * Internal helper for the resolution functions.
	 */
	static void GetBlackboardSettingsFromFlowAsset(
		const UFlowAsset* FlowAsset,
		TSubclassOf<UBlackboardComponent>& OutBlackboardComponentClass,
		UBlackboardData*& OutDefaultBlackboardAsset
	);
};