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

	return FAIFlowActorBlackboardHelper::FindOrAddBlackboardComponentOnActor(
		*Actor,
		InjectComponentsManager,
		BlackboardComponentClass,
		DesiredBlackboardAsset,
		SearchRule,
		InjectRule);
}

TArray<UBlackboardComponent*> FFlowNodeBlackboardUtils::ResolveBlackboardComponentsForActors(
	const TArray<AActor*>& Actors,
	const UFlowAsset* FlowAsset,
	UFlowInjectComponentsManager* InjectComponentsManager,
	UBlackboardData* SpecificBlackboardAsset,
	EActorBlackboardSearchRule SearchRule,
	EActorBlackboardInjectRule InjectRule)
{
	TSubclassOf<UBlackboardComponent> BlackboardComponentClass;
	UBlackboardData* DefaultBlackboardAsset = nullptr;
	GetBlackboardSettingsFromFlowAsset(FlowAsset, BlackboardComponentClass, DefaultBlackboardAsset);

	// Use specific blackboard asset if provided, otherwise fall back to flow asset default
	UBlackboardData* DesiredBlackboardAsset = SpecificBlackboardAsset ? SpecificBlackboardAsset : DefaultBlackboardAsset;

	return FAIFlowActorBlackboardHelper::FindOrAddBlackboardComponentOnActors(
		Actors,
		InjectComponentsManager,
		BlackboardComponentClass,
		DesiredBlackboardAsset,
		SearchRule,
		InjectRule);
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