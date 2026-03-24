// Copyright https://github.com/MothCocoon/FlowGraph/graphs/contributors

#include "Misc/AutomationTest.h"

#include "AddOns/FlowNodeAddOn_PredicateCompareBlackboardValue.h"
#include "Blackboard/FlowBlackboardEntryValue_Float.h"
#include "Blackboard/FlowBlackboardEntryValue_Int.h"

// ---------------------------------------------------------------------------
// GetOperatorSymbolString
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowPredicateOperatorSymbols,
	"AIFlow.Predicate.GetOperatorSymbolString",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowPredicateOperatorSymbols::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Equal symbol"), GetOperatorSymbolString(EPredicateCompareOperatorType::Equal), FString(TEXT("==")));
	TestEqual(TEXT("NotEqual symbol"), GetOperatorSymbolString(EPredicateCompareOperatorType::NotEqual), FString(TEXT("!=")));
	TestEqual(TEXT("Less symbol"), GetOperatorSymbolString(EPredicateCompareOperatorType::Less), FString(TEXT("<")));
	TestEqual(TEXT("LessOrEqual symbol"), GetOperatorSymbolString(EPredicateCompareOperatorType::LessOrEqual), FString(TEXT("<=")));
	TestEqual(TEXT("Greater symbol"), GetOperatorSymbolString(EPredicateCompareOperatorType::Greater), FString(TEXT(">")));
	TestEqual(TEXT("GreaterOrEqual symbol"), GetOperatorSymbolString(EPredicateCompareOperatorType::GreaterOrEqual), FString(TEXT(">=")));

	return true;
}

// ---------------------------------------------------------------------------
// IsEqualityOperation / IsArithmeticOperation classification
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowPredicateOperatorClassification,
	"AIFlow.Predicate.OperatorClassification",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowPredicateOperatorClassification::RunTest(const FString& Parameters)
{
	// Equality operators
	TestTrue(TEXT("Equal is equality"), EPredicateCompareOperatorType::Equal >= EPredicateCompareOperatorType::EqualityFirst && EPredicateCompareOperatorType::Equal <= EPredicateCompareOperatorType::EqualityLast);
	TestTrue(TEXT("NotEqual is equality"), EPredicateCompareOperatorType::NotEqual >= EPredicateCompareOperatorType::EqualityFirst && EPredicateCompareOperatorType::NotEqual <= EPredicateCompareOperatorType::EqualityLast);

	// Arithmetic operators
	TestTrue(TEXT("Less is arithmetic"), EPredicateCompareOperatorType::Less >= EPredicateCompareOperatorType::ArithmeticFirst && EPredicateCompareOperatorType::Less <= EPredicateCompareOperatorType::ArithmeticLast);
	TestTrue(TEXT("LessOrEqual is arithmetic"), EPredicateCompareOperatorType::LessOrEqual >= EPredicateCompareOperatorType::ArithmeticFirst && EPredicateCompareOperatorType::LessOrEqual <= EPredicateCompareOperatorType::ArithmeticLast);
	TestTrue(TEXT("Greater is arithmetic"), EPredicateCompareOperatorType::Greater >= EPredicateCompareOperatorType::ArithmeticFirst && EPredicateCompareOperatorType::Greater <= EPredicateCompareOperatorType::ArithmeticLast);
	TestTrue(TEXT("GreaterOrEqual is arithmetic"), EPredicateCompareOperatorType::GreaterOrEqual >= EPredicateCompareOperatorType::ArithmeticFirst && EPredicateCompareOperatorType::GreaterOrEqual <= EPredicateCompareOperatorType::ArithmeticLast);

	// Cross-check: equality ops are NOT arithmetic, arithmetic ops are NOT equality
	TestFalse(TEXT("Equal is not arithmetic"), EPredicateCompareOperatorType::Equal >= EPredicateCompareOperatorType::ArithmeticFirst && EPredicateCompareOperatorType::Equal <= EPredicateCompareOperatorType::ArithmeticLast);
	TestFalse(TEXT("Less is not equality"), EPredicateCompareOperatorType::Less >= EPredicateCompareOperatorType::EqualityFirst && EPredicateCompareOperatorType::Less <= EPredicateCompareOperatorType::EqualityLast);

	return true;
}

// ---------------------------------------------------------------------------
// EPredicateCompareOperatorType enum range consistency
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowPredicateEnumRange,
	"AIFlow.Predicate.EnumRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowPredicateEnumRange::RunTest(const FString& Parameters)
{
	// Max should be 6 (6 operators: Equal, NotEqual, Less, LessOrEqual, Greater, GreaterOrEqual)
	TestEqual(TEXT("Max should be 6"), static_cast<int32>(EPredicateCompareOperatorType::Max), 6);
	TestEqual(TEXT("Min should be 0"), static_cast<int32>(EPredicateCompareOperatorType::Min), 0);

	// Equality subrange
	TestEqual(TEXT("EqualityFirst is Equal"), EPredicateCompareOperatorType::EqualityFirst, EPredicateCompareOperatorType::Equal);
	TestEqual(TEXT("EqualityLast is NotEqual"), EPredicateCompareOperatorType::EqualityLast, EPredicateCompareOperatorType::NotEqual);

	// Arithmetic subrange
	TestEqual(TEXT("ArithmeticFirst is Less"), EPredicateCompareOperatorType::ArithmeticFirst, EPredicateCompareOperatorType::Less);
	TestEqual(TEXT("ArithmeticLast is GreaterOrEqual"), EPredicateCompareOperatorType::ArithmeticLast, EPredicateCompareOperatorType::GreaterOrEqual);

	return true;
}

// ---------------------------------------------------------------------------
// Float entry supports arithmetic, Bool does not
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowPredicateArithmeticSupport,
	"AIFlow.Predicate.ArithmeticSupport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowPredicateArithmeticSupport::RunTest(const FString& Parameters)
{
	// Float supports arithmetic
	{
		UFlowBlackboardEntryValue_Float* FloatEntry = NewObject<UFlowBlackboardEntryValue_Float>();
		int32 IntVal = 0;
		float FloatVal = 0.0f;
		TestTrue(TEXT("Float supports arithmetic"), FloatEntry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal));
	}

	// Int supports arithmetic
	{
		UFlowBlackboardEntryValue_Int* IntEntry = NewObject<UFlowBlackboardEntryValue_Int>();
		int32 IntVal = 0;
		float FloatVal = 0.0f;
		TestTrue(TEXT("Int supports arithmetic"), IntEntry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal));
	}

	return true;
}

// ---------------------------------------------------------------------------
// Float arithmetic: boundary values
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowPredicateFloatArithmeticBoundary,
	"AIFlow.Predicate.Float.ArithmeticBoundaryValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowPredicateFloatArithmeticBoundary::RunTest(const FString& Parameters)
{
	// Zero
	{
		UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
		// FloatValue defaults to 0.0f

		int32 IntVal = -1;
		float FloatVal = -1.0f;
		Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal);
		TestEqual(TEXT("Zero float -> int 0"), IntVal, 0);
		TestEqual(TEXT("Zero float -> float 0"), FloatVal, 0.0f);
	}

	// Negative value
	{
		UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
		FFloatProperty* Prop = CastField<FFloatProperty>(UFlowBlackboardEntryValue_Float::StaticClass()->FindPropertyByName(TEXT("FloatValue")));
		Prop->SetPropertyValue_InContainer(Entry, -3.7f);

		int32 IntVal = 0;
		float FloatVal = 0.0f;
		Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal);
		TestEqual(TEXT("Negative float -> int floor(-3.7) = -4"), IntVal, -4);
		TestEqual(TEXT("Negative float -> float -3.7"), FloatVal, -3.7f);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Int arithmetic: boundary values
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowPredicateIntArithmeticBoundary,
	"AIFlow.Predicate.Int.ArithmeticBoundaryValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowPredicateIntArithmeticBoundary::RunTest(const FString& Parameters)
{
	// Zero
	{
		UFlowBlackboardEntryValue_Int* Entry = NewObject<UFlowBlackboardEntryValue_Int>();
		// IntValue defaults to 0

		int32 IntVal = -1;
		float FloatVal = -1.0f;
		Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal);
		TestEqual(TEXT("Zero int -> int 0"), IntVal, 0);
		TestEqual(TEXT("Zero int -> float 0"), FloatVal, 0.0f);
	}

	// Negative value
	{
		UFlowBlackboardEntryValue_Int* Entry = NewObject<UFlowBlackboardEntryValue_Int>();
		FIntProperty* Prop = CastField<FIntProperty>(UFlowBlackboardEntryValue_Int::StaticClass()->FindPropertyByName(TEXT("IntValue")));
		Prop->SetPropertyValue_InContainer(Entry, -10);

		int32 IntVal = 0;
		float FloatVal = 0.0f;
		Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, &FloatVal);
		TestEqual(TEXT("Negative int -> int -10"), IntVal, -10);
		TestEqual(TEXT("Negative int -> float -10.0"), FloatVal, -10.0f);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Null output pointers for TryGetNumericalValues (partial output)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowPredicateArithmeticNullOutput,
	"AIFlow.Predicate.ArithmeticNullOutputPointers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowPredicateArithmeticNullOutput::RunTest(const FString& Parameters)
{
	UFlowBlackboardEntryValue_Float* Entry = NewObject<UFlowBlackboardEntryValue_Float>();
	{
		FFloatProperty* Prop = CastField<FFloatProperty>(UFlowBlackboardEntryValue_Float::StaticClass()->FindPropertyByName(TEXT("FloatValue")));
		Prop->SetPropertyValue_InContainer(Entry, 5.0f);
	}

	// Only request int
	{
		int32 IntVal = 0;
		const bool bResult = Entry->TryGetNumericalValuesForArithmeticOperation(&IntVal, nullptr);
		TestTrue(TEXT("Should succeed with null float output"), bResult);
		TestEqual(TEXT("IntVal should be 5"), IntVal, 5);
	}

	// Only request float
	{
		float FloatVal = 0.0f;
		const bool bResult = Entry->TryGetNumericalValuesForArithmeticOperation(nullptr, &FloatVal);
		TestTrue(TEXT("Should succeed with null int output"), bResult);
		TestEqual(TEXT("FloatVal should be 5.0"), FloatVal, 5.0f);
	}

	// Both null — just check it does not crash
	{
		const bool bResult = Entry->TryGetNumericalValuesForArithmeticOperation(nullptr, nullptr);
		TestTrue(TEXT("Should succeed with both null outputs"), bResult);
	}

	return true;
}

// ---------------------------------------------------------------------------
// FlowBlackboardEntry equality operators
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFlowBlackboardEntryEquality,
	"AIFlow.Predicate.FlowBlackboardEntry.Equality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIFlowBlackboardEntryEquality::RunTest(const FString& Parameters)
{
	FFlowBlackboardEntry A;
	A.KeyName = TEXT("Health");

	FFlowBlackboardEntry B;
	B.KeyName = TEXT("Health");

	FFlowBlackboardEntry C;
	C.KeyName = TEXT("Armor");

	TestTrue(TEXT("Same key names should be equal"), A == B);
	TestFalse(TEXT("Different key names should not be equal"), A == C);
	TestTrue(TEXT("Different key names via != operator"), A != C);
	TestFalse(TEXT("Same key names via != operator"), A != B);

	return true;
}
