# Blackboard Integration Deep Dive

This document supplements the `README.md` examples with the technical details needed to correctly integrate AI flows with Unreal's AI Blackboard system.

---

## Blackboard Setup Requirements

### What the AI Controller must provide

An AI flow's blackboard nodes operate on the **subject actor's AI Controller**. The Controller must:

1. Have a `UAIController` (or subclass) actively possessing the pawn.
2. Have called `UseBlackboard(UBlackboardData*, UBlackboardComponent**)` so that `GetBlackboard()` returns a valid component.
3. Have all keys that the flow intends to read/write declared in the `UBlackboardData` asset.

If any of these is missing, `FlowNode_SetBlackboardValues` and `FlowNode_GetBlackboardValues` will silently no-op (they null-check internally). Use `FlowNode_EnsureActorHasBlackboard` to get an explicit error log and early-out path when the BB is absent.

### Recommended blackboard asset layout

Create one `UBlackboardData` asset per AI archetype (e.g., `BB_Patrol`, `BB_Guard`). Inherit from a base `BB_AIBase` that holds keys every AI shares:

```
BB_AIBase
├── TargetActor         (Object / AActor)
├── ThreatLevel         (Int)
├── LastKnownLocation   (Vector)
└── AlertState          (Enum / Name)

BB_Patrol  (parent: BB_AIBase)
└── PatrolIndex         (Int)

BB_Guard   (parent: BB_AIBase)
└── GuardPostLocation   (Vector)
```

Flows that reference only `BB_AIBase` keys will work with any archetype — write flows against the base BB where possible.

---

## Available Blackboard Node Types

### `FlowNode_SetBlackboardValues`

Sets one or more BB keys to **literal constant values**. Use for initialization and reset.

```
Details panel:
  Values (array):
    [0] Key = "PatrolIndex", bIsInt = true, IntValue = 0
    [1] Key = "AlertState",  bIsName = true, NameValue = "Idle"
```

**Execution**: synchronous, completes immediately on the same tick.

---

### `FlowNode_SetBlackboardValuesV2`

Like `SetBlackboardValues` but supports **in-place arithmetic** on numeric keys:

| Operation | Effect |
|-----------|--------|
| `Set`       | `Key = Value` (same as V1) |
| `Increment` | `Key += Value` (default Value=1) |
| `Decrement` | `Key -= Value` |
| `Multiply`  | `Key *= Value` |
| `Clamp`     | `Key = Clamp(Key, Min, Max)` |

Use `Increment` to advance a patrol index without a separate Get node.

---

### `FlowNode_SetBlackboardValuesOnActor`

Identical interface to `SetBlackboardValues` but targets a **specific actor** passed via an input object pin rather than the flow's implicit subject actor. Use when one AI needs to write to another AI's blackboard (e.g., a squad leader signalling squad members).

---

### `FlowNode_GetBlackboardValues`

Reads one or more BB keys and **exposes them as output data pins** that downstream nodes can consume. In practice most decisions are better handled by the `PredicateCompareBlackboardValue` add-on — use `GetBlackboardValues` only when you need the raw value for a custom node or a Blueprint-callable function.

---

### `FlowNode_EnsureActorHasBlackboard`

**Guard node** — has two output execution pins:

- `HasBlackboard` — fires when the subject actor's AI Controller has a valid BB component.
- `NoBlackboard` — fires otherwise, letting you gracefully skip the rest of the flow or trigger a fallback.

Place this immediately after `Start` in any flow that reads or writes the blackboard.

---

### `FlowNodeAddOn_PredicateCompareBlackboardValue`

An **add-on** (not a standalone node) that attaches to a host node implementing `IFlowPredicateInterface` — primarily `FlowNode_Branch` and `FlowNode_Gate`.

Configuration:

| Property | Description |
|----------|-------------|
| `Key` | Blackboard key name to evaluate |
| `CompareType` | `EqualTo`, `NotEqualTo`, `LessThan`, `LessThanOrEqualTo`, `GreaterThan`, `GreaterThanOrEqualTo` |
| `Value` | Constant to compare against (typed to match the key) |

The predicate is evaluated **when the host node's In pin fires**, before the host decides which output pin to activate. Multiple add-ons on the same host are AND-ed together by default.

---

## How AI Flows Interact with Behavior Trees

### Two models of co-existence

#### Model A — Flow as a BT Task replacement

```
BehaviorTree
└── Selector
    └── Task_RunFlowAsset (custom BTTask)  ← entire behavior in a flow
```

The BT task starts the flow, awaits `FlowNode_Finish`, then reports success/failure back to the BT. The flow handles all movement, animation, and blackboard updates. The BT provides priority/interruption logic.

#### Model B — Flow as a BT Service side-effect (recommended)

```
BehaviorTree
└── Selector
    ├── Sequence [attack]
    │   └── Service_AlertFlowLauncher  ← monitors BB, starts FA_AlertResponse once
    └── Task_Patrol (runs FA_SimplePatrol)
```

A BT service watches for a condition (e.g., `ThreatLevel > 0`), fires a flow once, and lets the BT continue running independently. The flow finishes asynchronously and updates the BB. The BT's next tick reads the updated BB and replans.

Model B is preferred because the BT retains full reactive control — the flow cannot block BT re-evaluation.

---

## AI Flows vs Regular FlowGraph Flows

| Feature | FlowGraph (base) | AIFlowGraph |
|---------|-----------------|-------------|
| Subject actor | Generic `Owner` actor | AI Controller pawn |
| State storage | None (stateless) | AI Blackboard |
| Decision nodes | Counter, random, custom | Blackboard predicates, weighted rolls |
| Event source | Any `FGameplayTag` event | AI perception events, BT signals |
| Lifetime management | Manual (Start/Stop asset) | BT task or AI Controller lifetime |
| Typical graph size | 5–30 nodes (cutscenes, UI) | 3–15 nodes (micro-behaviors) |
| Interruption model | External Stop call | BT abort signals propagate to active flows |

**Rule of thumb**: if the logic needs to *react to AI perception data or BT blackboard values*, use an AI flow. If the logic is purely event-driven sequencing (cutscenes, puzzle triggers, dialog), use a plain FlowGraph flow.

---

## Common Mistakes

### Writing to a key that doesn't exist in the blackboard asset

`SetBlackboardValues` silently fails. Always verify the key is declared in the `UBlackboardData` asset assigned to the AI Controller. Add a `FlowNode_Log` immediately after `SetBlackboardValues` during development to confirm the node executed.

### Reading stale values after async nodes

`FlowNode_Timer` and `FlowNode_WaitForGameplayEvent` have nonzero latency. If the BT modifies a BB key while the flow is waiting, the flow will read the *post-wait* value on its next `GetBlackboardValues`. This is usually correct (you want fresh data), but be aware of it when debugging unexpected branches.

### Assuming `EnsureActorHasBlackboard` creates the blackboard

It does **not** create one — it only checks existence. Blackboard creation is the AI Controller's responsibility (call `UseBlackboard` in `OnPossess`).

### Chaining too many SetBlackboardValues nodes

Each set node takes one execution step. For bulk initialization, consolidate all initial key writes into a single `SetBlackboardValues` node with multiple entries instead of chaining several nodes.
