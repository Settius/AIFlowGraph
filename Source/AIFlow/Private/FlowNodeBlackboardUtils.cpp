// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "FlowNodeBlackboardUtils.h"
#include "AIFlowAsset.h"
#include "BehaviorTree/BlackboardComponent.h"

UBlackboardComponent* FFlowNodeBlackboardUtils::ResolveBlackboardComponentForActor(
	AActor* Actor,
	const UFlowAsset* FlowAsset,
	UFlowInjectComponentsManager* InjectComponentsManager,
	UBlackboardData* SpecificBlackboardAsset,
	EActorBlackboardSearchRule SearchRule,
	EActorBlackboardInjectRule InjectRule)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	TSubclassOf<UBlackboardComponent> BlackboardComponentClass;
	UBlackboardData* DefaultBlackboardAsset = nullptr;
	GetBlackboardSettingsFromFlowAsset(FlowAsset, BlackboardComponentClass, DefaultBlackboardAsset);

	// Use specific blackboard asset if provided, otherwise fall back to flow asset default
	UBlackboardData* DesiredBlackboardAsset = SpecificBlackboardAsset ? SpecificBlackboardAsset : DefaultBlackboardAsset;

	// ST-165: Check the flow asset's per-actor blackboard lookup cache first
	const UAIFlowAsset* AIFlowAsset = Cast<UAIFlowAsset>(FlowAsset);
	if (AIFlowAsset)
	{
		if (UBlackboardComponent* CachedComponent = AIFlowAsset->FindCachedBlackboardForActor(Actor, DesiredBlackboardAsset))
		{
			return CachedComponent;
		}
	}

	UBlackboardComponent* Result = FAIFlowActorBlackboardHelper::FindOrAddBlackboardComponentOnActor(
		*Actor,
		InjectComponentsManager,
		BlackboardComponentClass,
		DesiredBlackboardAsset,
		SearchRule,
		InjectRule);

	// ST-165: Cache the result for subsequent lookups in the same flow execution
	if (Result && AIFlowAsset)
	{
		AIFlowAsset->CacheBlackboardForActor(Actor, DesiredBlackboardAsset, Result);
	}

	return Result;
}

TArray<UBlackboardComponent*> FFlowNodeBlackboardUtils::ResolveBlackboardComponentsForActors(
	const TArray<AActor*>& Actors,
	const UFlowAsset* FlowAsset,
	UFlowInjectComponentsManager* InjectComponentsManager,
	UBlackboardData* SpecificBlackboardAsset,
	EActorBlackboardSearchRule SearchRule,
	EActorBlackboardInjectRule InjectRule)
{
	// ST-165: Route each actor through the caching single-actor path so that
	// repeated lookups for the same actor benefit from the per-flow-instance cache.
	TArray<UBlackboardComponent*> BlackboardComponents;
	BlackboardComponents.Reserve(Actors.Num());

	for (AActor* Actor : Actors)
	{
		UBlackboardComponent* Component = ResolveBlackboardComponentForActor(
			Actor,
			FlowAsset,
			InjectComponentsManager,
			SpecificBlackboardAsset,
			SearchRule,
			InjectRule);

		if (Component)
		{
			BlackboardComponents.Add(Component);
		}
	}

	return BlackboardComponents;
}

#if WITH_EDITOR
UBlackboardData* FFlowNodeBlackboardUtils::GetBlackboardAssetForEditor(
	UBlackboardData* SpecificBlackboardAsset,
	const UFlowAsset* FlowAsset)
{
	if (IsValid(SpecificBlackboardAsset))
	{
		return SpecificBlackboardAsset;
	}

	// Fall back to flow asset's default blackboard
	if (const UAIFlowAsset* AIFlowAsset = Cast<UAIFlowAsset>(FlowAsset))
	{
		return AIFlowAsset->GetBlackboardAsset();
	}

	return nullptr;
}
#endif // WITH_EDITOR

void FFlowNodeBlackboardUtils::GetBlackboardSettingsFromFlowAsset(
	const UFlowAsset* FlowAsset,
	TSubclassOf<UBlackboardComponent>& OutBlackboardComponentClass,
	UBlackboardData*& OutDefaultBlackboardAsset)
{
	OutBlackboardComponentClass = UBlackboardComponent::StaticClass();
	OutDefaultBlackboardAsset = nullptr;

	if (const UAIFlowAsset* AIFlowAsset = Cast<UAIFlowAsset>(FlowAsset))
	{
		OutBlackboardComponentClass = AIFlowAsset->GetBlackboardComponentClass();
		OutDefaultBlackboardAsset = AIFlowAsset->GetBlackboardAsset();
	}
}