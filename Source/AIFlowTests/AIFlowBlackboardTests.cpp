// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Misc/AutomationTest.h"

#include "Blackboard/FlowBlackboardEntryValue_Bool.h"
#include "Blackboard/FlowBlackboardEntryValue_Float.h"
#include "Blackboard/FlowBlackboardEntryValue_Int.h"
#include "Blackboard/FlowBlackboardEntryValue_String.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_String.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Tests/AutomationCommon.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace AIFlowTestHelpers
{
	/** Create a minimal UBlackboardData with the requested keys. */
	UBlackboardData* CreateBlackboardData(UObject* Outer, const TMap<FName, TSubclassOf<UBlackboardKeyType>>& Keys)
	{
		UBlackboardData* BB = NewObject<UBlackboardData>(Outer);

		for (const auto& Pair : Keys)
		{
			FBlackboardEntry Entry;
			Entry.EntryName = Pair.Key;
			Entry.KeyType = NewObject<UBlackboardKeyType>(BB, Pair.Value);
			BB->Keys.Add(MoveTemp(Entry));
		}

		return BB;
	}

	/** Attach a UBlackboardComponent to an actor and initialise it with the given data. */
	UBlackboardComponent* CreateBlackboardComponent(AActor* Owner, UBlackboardData* BBData)
	{
		UBlackboardComponent* Comp = NewObject<UBlackboardComponent>(Owner);
		Comp->RegisterComponent();
		Comp->InitializeBlackboard(*BBData);
		return Comp;
	}

	/** Spawn a transient actor in the given world. */
	AActor* SpawnTransientActor(UWorld* World)
	{
		FActorSpawnParameters Params;
		return World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
	}
}

// ---------------------------------------------------------------------------
// Float entry value — SetOnBlackboardComponent
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryFloatSet,
	"AIFlow.Blackboard.EntryValue.Float.SetOnBlackboardComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryFloatSet::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("Health"), UBlackboardKeyType_Float::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	// Create entry value and set it
	UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
	Entry->Key.KeyName = TEXT("Health");

	// Access the float value through the public UPROPERTY via pointer offset
	// Since FloatValue is protected, we set it via CDO manipulation
	// Actually, let's use the fact that SetOnBlackboardComponent reads the internal FloatValue.
	// We need to set FloatValue — it's a protected UPROPERTY so we can use FindPropertyByName.
	{
		FFloatProperty* Prop = CastField<FFloatProperty>(UFlowBlackboardEntryValue_Float::StaticClass()->FindPropertyByName(TEXT("FloatValue")));
		check(Prop);
		Prop->SetPropertyValue_InContainer(Entry, 42.5f);
	}

	Entry->SetOnBlackboardComponent(Comp);

	const float Result = Comp->GetValueAsFloat(TEXT("Health"));
	TestEqual(TEXT("Float value should be 42.5"), Result, 42.5f);

	return true;
}

// ---------------------------------------------------------------------------
// Float entry value — CompareKeyValues (Equal)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryFloatCompareEqual,
	"AIFlow.Blackboard.EntryValue.Float.CompareKeyValues.Equal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryFloatCompareEqual::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("Score"), UBlackboardKeyType_Float::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	Comp->SetValueAsFloat(TEXT("Score"), 10.0f);

	UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
	{
		FFloatProperty* Prop = CastField<FFloatProperty>(UFlowBlackboardEntryValue_Float::StaticClass()->FindPropertyByName(TEXT("FloatValue")));
		Prop->SetPropertyValue_InContainer(Entry, 10.0f);
	}

	const EBlackboardCompare::Type CompareResult = Entry->CompareKeyValues(Comp, TEXT("Score"));
	TestTrue(TEXT("Matching floats should compare as Equal"), CompareResult == EBlackboardCompare::Equal);

	return true;
}

// ---------------------------------------------------------------------------
// Float entry value — CompareKeyValues (NotEqual)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryFloatCompareNotEqual,
	"AIFlow.Blackboard.EntryValue.Float.CompareKeyValues.NotEqual",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryFloatCompareNotEqual::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("Score"), UBlackboardKeyType_Float::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	Comp->SetValueAsFloat(TEXT("Score"), 10.0f);

	UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
	{
		FFloatProperty* Prop = CastField<FFloatProperty>(UFlowBlackboardEntryValue_Float::StaticClass()->FindPropertyByName(TEXT("FloatValue")));
		Prop->SetPropertyValue_InContainer(Entry, 99.0f);
	}

	const EBlackboardCompare::Type CompareResult = Entry->CompareKeyValues(Comp, TEXT("Score"));
	TestTrue(TEXT("Mismatched floats should compare as NotEqual"), CompareResult == EBlackboardCompare::NotEqual);

	return true;
}

// ---------------------------------------------------------------------------
// Float entry value — TryGetNumericalValuesForArithmeticOperation
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryFloatArithmetic,
	"AIFlow.Blackboard.EntryValue.Float.TryGetNumericalValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryFloatArithmetic::RunTest(const FString& Parameters)
{
	UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
	{
		FFloatProperty* Prop = CastField<FFloatProperty>(UFlowBlackboardEntryValue_Float::StaticClass()->FindPropertyByName(TEXT("FloatValue")));
		Prop->SetPropertyValue_InContainer(Entry, 7.5f);
	}

	int32 IntVal = 0;
	float FloatVal = 0.0f;
	const bool bResult = Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal);

	TestTrue(TEXT("Float entry should support arithmetic"), bResult);
	TestEqual(TEXT("FloatVal should be 7.5"), FloatVal, 7.5f);
	TestEqual(TEXT("IntVal should be floor(7.5) = 7"), IntVal, 7);

	return true;
}

// ---------------------------------------------------------------------------
// Float entry value — GetSupportedBlackboardKeyType
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryFloatKeyType,
	"AIFlow.Blackboard.EntryValue.Float.GetSupportedBlackboardKeyType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryFloatKeyType::RunTest(const FString& Parameters)
{
	UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
	TestTrue(TEXT("Supported key type should be Float"), Entry->GetSupportedBlackboardKeyType() == UBlackboardKeyType_Float::StaticClass());
	return true;
}

// ---------------------------------------------------------------------------
// Int entry value — SetOnBlackboardComponent
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryIntSet,
	"AIFlow.Blackboard.EntryValue.Int.SetOnBlackboardComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryIntSet::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("Ammo"), UBlackboardKeyType_Int::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	UFlowBlackboardEntryValue_Int* Entry = NewObject<UFlowBlackboardEntryValue_Int>();
	Entry->Key.KeyName = TEXT("Ammo");
	{
		FIntProperty* Prop = CastField<FIntProperty>(UFlowBlackboardEntryValue_Int::StaticClass()->FindPropertyByName(TEXT("IntValue")));
		Prop->SetPropertyValue_InContainer(Entry, 100);
	}

	Entry->SetOnBlackboardComponent(Comp);

	const int32 Result = Comp->GetValueAsInt(TEXT("Ammo"));
	TestEqual(TEXT("Int value should be 100"), Result, 100);

	return true;
}

// ---------------------------------------------------------------------------
// Int entry value — CompareKeyValues
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryIntCompare,
	"AIFlow.Blackboard.EntryValue.Int.CompareKeyValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryIntCompare::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("Ammo"), UBlackboardKeyType_Int::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	Comp->SetValueAsInt(TEXT("Ammo"), 50);

	UFlowBlackboardEntryValue_Int* Entry = NewObject<UFlowBlackboardEntryValue_Int>();
	{
		FIntProperty* Prop = CastField<FIntProperty>(UFlowBlackboardEntryValue_Int::StaticClass()->FindPropertyByName(TEXT("IntValue")));
		Prop->SetPropertyValue_InContainer(Entry, 50);
	}

	TestTrue(TEXT("Matching ints should be Equal"), Entry->CompareKeyValues(Comp, TEXT("Ammo")) == EBlackboardCompare::Equal);

	// Mismatch
	{
		FIntProperty* Prop = CastField<FIntProperty>(UFlowBlackboardEntryValue_Int::StaticClass()->FindPropertyByName(TEXT("IntValue")));
		Prop->SetPropertyValue_InContainer(Entry, 99);
	}
	TestTrue(TEXT("Mismatched ints should be NotEqual"), Entry->CompareKeyValues(Comp, TEXT("Ammo")) == EBlackboardCompare::NotEqual);

	return true;
}

// ---------------------------------------------------------------------------
// Int entry value — TryGetNumericalValuesForArithmeticOperation
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryIntArithmetic,
	"AIFlow.Blackboard.EntryValue.Int.TryGetNumericalValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryIntArithmetic::RunTest(const FString& Parameters)
{
	UFlowBlackboardEntryValue_Int* Entry = NewObject<UFlowBlackboardEntryValue_Int>();
	{
		FIntProperty* Prop = CastField<FIntProperty>(UFlowBlackboardEntryValue_Int::StaticClass()->FindPropertyByName(TEXT("IntValue")));
		Prop->SetPropertyValue_InContainer(Entry, 42);
	}

	int32 IntVal = 0;
	float FloatVal = 0.0f;
	const bool bResult = Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal);

	TestTrue(TEXT("Int entry should support arithmetic"), bResult);
	TestEqual(TEXT("IntVal should be 42"), IntVal, 42);
	TestEqual(TEXT("FloatVal should be 42.0"), FloatVal, 42.0f);

	return true;
}

// ---------------------------------------------------------------------------
// Bool entry value — SetOnBlackboardComponent
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryBoolSet,
	"AIFlow.Blackboard.EntryValue.Bool.SetOnBlackboardComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryBoolSet::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("IsAlive"), UBlackboardKeyType_Bool::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	UFlowBlackboardEntryValue_Bool* Entry = NewObject<UFlowBlackboardEntryValue_Bool>();
	Entry->Key.KeyName = TEXT("IsAlive");
	{
		FBoolProperty* Prop = CastField<FBoolProperty>(UFlowBlackboardEntryValue_Bool::StaticClass()->FindPropertyByName(TEXT("bBoolValue")));
		Prop->SetPropertyValue_InContainer(Entry, true);
	}

	Entry->SetOnBlackboardComponent(Comp);

	const bool bResult = Comp->GetValueAsBool(TEXT("IsAlive"));
	TestTrue(TEXT("Bool value should be true"), bResult);

	return true;
}

// ---------------------------------------------------------------------------
// Bool entry value — CompareKeyValues
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryBoolCompare,
	"AIFlow.Blackboard.EntryValue.Bool.CompareKeyValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryBoolCompare::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("IsAlive"), UBlackboardKeyType_Bool::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	Comp->SetValueAsBool(TEXT("IsAlive"), true);

	UFlowBlackboardEntryValue_Bool* Entry = NewObject<UFlowBlackboardEntryValue_Bool>();
	{
		FBoolProperty* Prop = CastField<FBoolProperty>(UFlowBlackboardEntryValue_Bool::StaticClass()->FindPropertyByName(TEXT("bBoolValue")));
		Prop->SetPropertyValue_InContainer(Entry, true);
	}

	TestTrue(TEXT("Matching bools should be Equal"), Entry->CompareKeyValues(Comp, TEXT("IsAlive")) == EBlackboardCompare::Equal);

	// Mismatch
	{
		FBoolProperty* Prop = CastField<FBoolProperty>(UFlowBlackboardEntryValue_Bool::StaticClass()->FindPropertyByName(TEXT("bBoolValue")));
		Prop->SetPropertyValue_InContainer(Entry, false);
	}
	TestTrue(TEXT("Mismatched bools should be NotEqual"), Entry->CompareKeyValues(Comp, TEXT("IsAlive")) == EBlackboardCompare::NotEqual);

	return true;
}

// ---------------------------------------------------------------------------
// Bool entry value — does NOT support arithmetic
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryBoolNoArithmetic,
	"AIFlow.Blackboard.EntryValue.Bool.DoesNotSupportArithmetic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryBoolNoArithmetic::RunTest(const FString& Parameters)
{
	UFlowBlackboardEntryValue_Bool* Entry = NewObject<UFlowBlackboardEntryValue_Bool>();
	int32 IntVal = 0;
	float FloatVal = 0.0f;
	const bool bResult = Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal);
	TestFalse(TEXT("Bool entry should NOT support arithmetic"), bResult);
	return true;
}

// ---------------------------------------------------------------------------
// String entry value — SetOnBlackboardComponent
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryStringSet,
	"AIFlow.Blackboard.EntryValue.String.SetOnBlackboardComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryStringSet::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("Name"), UBlackboardKeyType_String::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	UFlowBlackboardEntryValue_String* Entry = NewObject<UFlowBlackboardEntryValue_String>();
	Entry->Key.KeyName = TEXT("Name");
	{
		FStrProperty* Prop = CastField<FStrProperty>(UFlowBlackboardEntryValue_String::StaticClass()->FindPropertyByName(TEXT("StringValue")));
		check(Prop);
		*Prop->ContainerPtrToValuePtr<FString>(Entry) = TEXT("TestName");
	}

	Entry->SetOnBlackboardComponent(Comp);

	const FString Result = Comp->GetValueAsString(TEXT("Name"));
	TestEqual(TEXT("String value should be 'TestName'"), Result, FString(TEXT("TestName")));

	return true;
}

// ---------------------------------------------------------------------------
// String entry value — CompareKeyValues
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryStringCompare,
	"AIFlow.Blackboard.EntryValue.String.CompareKeyValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryStringCompare::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

	AActor* Actor = AIFlowTestHelpers::SpawnTransientActor(World);

	TMap<FName, TSubclassOf<UBlackboardKeyType>> Keys;
	Keys.Add(TEXT("Name"), UBlackboardKeyType_String::StaticClass());
	UBlackboardData* BBData = AIFlowTestHelpers::CreateBlackboardData(GetTransientPackage(), Keys);
	UBlackboardComponent* Comp = AIFlowTestHelpers::CreateBlackboardComponent(Actor, BBData);

	Comp->SetValueAsString(TEXT("Name"), TEXT("Hello"));

	UFlowBlackboardEntryValue_String* Entry = NewObject<UFlowBlackboardEntryValue_String>();
	{
		FStrProperty* Prop = CastField<FStrProperty>(UFlowBlackboardEntryValue_String::StaticClass()->FindPropertyByName(TEXT("StringValue")));
		*Prop->ContainerPtrToValuePtr<FString>(Entry) = TEXT("Hello");
	}

	TestTrue(TEXT("Matching strings should be Equal"), Entry->CompareKeyValues(Comp, TEXT("Name")) == EBlackboardCompare::Equal);

	// Mismatch
	{
		FStrProperty* Prop = CastField<FStrProperty>(UFlowBlackboardEntryValue_String::StaticClass()->FindPropertyByName(TEXT("StringValue")));
		*Prop->ContainerPtrToValuePtr<FString>(Entry) = TEXT("World");
	}
	TestTrue(TEXT("Mismatched strings should be NotEqual"), Entry->CompareKeyValues(Comp, TEXT("Name")) == EBlackboardCompare::NotEqual);

	return true;
}

// ---------------------------------------------------------------------------
// CompareKeyValues with null blackboard returns NotEqual
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntryNullBlackboard,
	"AIFlow.Blackboard.EntryValue.CompareKeyValues.NullBlackboard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntryNullBlackboard::RunTest(const FString& Parameters)
{
	UFlowBlackboardEntryValue_Float* FloatEntry = NewObject<UFlowBlackboardEntryValue_Float>();
	TestTrue(TEXT("Float compare with null BB should be NotEqual"), FloatEntry->CompareKeyValues(nullptr, TEXT("Key")) == EBlackboardCompare::NotEqual);

	UFlowBlackboardEntryValue_Int* IntEntry = NewObject<UFlowBlackboardEntryValue_Int>();
	TestTrue(TEXT("Int compare with null BB should be NotEqual"), IntEntry->CompareKeyValues(nullptr, TEXT("Key")) == EBlackboardCompare::NotEqual);

	UFlowBlackboardEntryValue_Bool* BoolEntry = NewObject<UFlowBlackboardEntryValue_Bool>();
	TestTrue(TEXT("Bool compare with null BB should be NotEqual"), BoolEntry->CompareKeyValues(nullptr, TEXT("Key")) == EBlackboardCompare::NotEqual);

	UFlowBlackboardEntryValue_String* StringEntry = NewObject<UFlowBlackboardEntryValue_String>();
	TestTrue(TEXT("String compare with null BB should be NotEqual"), StringEntry->CompareKeyValues(nullptr, TEXT("Key")) == EBlackboardCompare::NotEqual);

	return true;
}

// ---------------------------------------------------------------------------
// SetOnBlackboardComponent with null component does not crash
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntrySetNullComp,
	"AIFlow.Blackboard.EntryValue.SetOnBlackboardComponent.NullComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntrySetNullComp::RunTest(const FString& Parameters)
{
	// All entry types should gracefully handle a null blackboard component
	UFlowBlackboardEntryValue_Float* FloatEntry = NewObject<UFlowBlackboardEntryValue_Float>();
	FloatEntry->SetOnBlackboardComponent(nullptr); // Should not crash

	UFlowBlackboardEntryValue_Int* IntEntry = NewObject<UFlowBlackboardEntryValue_Int>();
	IntEntry->SetOnBlackboardComponent(nullptr);

	UFlowBlackboardEntryValue_Bool* BoolEntry = NewObject<UFlowBlackboardEntryValue_Bool>();
	BoolEntry->SetOnBlackboardComponent(nullptr);

	UFlowBlackboardEntryValue_String* StringEntry = NewObject<UFlowBlackboardEntryValue_String>();
	StringEntry->SetOnBlackboardComponent(nullptr);

	// If we get here without a crash, the test passes
	TestTrue(TEXT("SetOnBlackboardComponent with null should not crash"), true);

	return true;
}

// ---------------------------------------------------------------------------
// GetSupportedBlackboardKeyType returns correct types
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBBEntrySupportedKeyTypes,
	"AIFlow.Blackboard.EntryValue.GetSupportedBlackboardKeyType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBBEntrySupportedKeyTypes::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Float key type"), NewObject<UFlowBlackboardEntryValue_Float>()->GetSupportedBlackboardKeyType() == UBlackboardKeyType_Float::StaticClass());
	TestTrue(TEXT("Int key type"), NewObject<UFlowBlackboardEntryValue_Int>()->GetSupportedBlackboardKeyType() == UBlackboardKeyType_Int::StaticClass());
	TestTrue(TEXT("Bool key type"), NewObject<UFlowBlackboardEntryValue_Bool>()->GetSupportedBlackboardKeyType() == UBlackboardKeyType_Bool::StaticClass());
	TestTrue(TEXT("String key type"), NewObject<UFlowBlackboardEntryValue_String>()->GetSupportedBlackboardKeyType() == UBlackboardKeyType_String::StaticClass());

	return true;
}
