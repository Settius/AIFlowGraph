# AIFlowGraph — Example AI Flows

This directory contains documentation and scaffolding for the two canonical AI flow examples. The actual `.uasset` files must be created in the Unreal Editor; this README explains every node, connection, and setting required to reconstruct them exactly.

---

## How AIFlowGraph Differs from FlowGraph

FlowGraph is a general-purpose visual scripting system — its flows are stateless event graphs that can run anywhere. **AIFlowGraph extends FlowGraph with AI-specific capabilities**:

| Concern | FlowGraph | AIFlowGraph |
|---------|-----------|-------------|
| Data access | No built-in actor state | Read/write AI Blackboard values |
| Decisions | Boolean predicates, counters | Blackboard comparisons (`PredicateCompareBlackboardValue`) |
| Actor awareness | Generic actor references | `EnsureActorHasBlackboard` guarantees BB exists before use |
| Spawning | Basic spawning | `ConfigureSpawnedActorBlackboard` wires BB on newly spawned actors |
| Execution branching | Counter/random | `ExecutionRollWeighted` / `ExecutionRollGuaranteed` for probabilistic AI choices |

**Key mental model**: an AI flow is owned by an AI Controller or a Behavior Tree service. It has a **subject actor** (the controlled pawn) whose Blackboard is accessible to every AI node in the graph. Regular FlowGraph flows have no concept of a subject actor or blackboard.

---

## FA_SimplePatrol

**Asset path**: `AIFlowGraph/Content/Examples/FA_SimplePatrol`  
**Purpose**: Demonstrate a blackboard-driven patrol loop — incrementing a patrol index each cycle and reading it back from the blackboard.

### Nodes required

| Step | Node class | Pin config |
|------|-----------|------------|
| 1 | `FlowNode_Start` | — |
| 2 | `FlowNode_SetBlackboardValues` | Key: `PatrolIndex`, Type: Int, Value: `0` |
| 3 | *(game-specific)* `FlowNode_MoveToLocation` or a custom `Log` placeholder | Input: BB key `PatrolIndex` (resolved to waypoint at runtime) |
| 4 | `FlowNode_Timer` | Duration: `2.0`, Mode: `Once` |
| 5 | `FlowNode_SetBlackboardValuesV2` | Key: `PatrolIndex`, Type: Int, Operation: `Increment` (or use a Counter node + SetBlackboardValues) |
| 6 | `FlowNode_Counter` | Limit: `MaxPatrolIndex` (match your waypoint array size) |
| 7 | `FlowNode_Finish` | — |

> **Note on step 5**: `SetBlackboardValuesV2` supports in-place math operations. If your version does not, chain a `GetBlackboardValues` → blueprint-computed increment → `SetBlackboardValues`.

### ASCII flow diagram

```
[Start]
   |
   v
[SetBlackboardValues]  ← PatrolIndex = 0 (reset/init)
   |
   v
[MoveToLocation]  ← reads PatrolIndex from BB to pick waypoint
   |
   v
[Timer (2 s)]
   |
   v
[SetBlackboardValuesV2]  ← PatrolIndex++
   |
   v
[Counter]
   |-- "Completed loop" --> [Finish]
   |-- "Still looping"  --> back to [MoveToLocation]
```

### How to build it in the editor

1. **Create** a new `FlowAsset` in `Content/Examples/`, name it `FA_SimplePatrol`.
2. **Add** `FlowNode_SetBlackboardValues` — open its details, add one entry: Key=`PatrolIndex`, bIsInt=true, IntValue=`0`.
3. **Add** `FlowNode_Timer` — set Duration=`2.0`, Mode=`Once`.
4. **Add** `FlowNode_SetBlackboardValuesV2` — set Key=`PatrolIndex`, Operation=`Increment`.
5. **Add** `FlowNode_Counter` — set Limit to your waypoint count.
6. Wire: Start → SetBlackboardValues → MoveToLocation → Timer → SetBlackboardValuesV2 → Counter.
7. Counter `Loop` output → MoveToLocation `In`. Counter `Completed` output → Finish.

---

## FA_AlertResponse

**Asset path**: `AIFlowGraph/Content/Examples/FA_AlertResponse`  
**Purpose**: Demonstrate event-driven AI escalation — waiting for an alert gameplay event, checking the blackboard threat level, then branching between an active behavior and an idle fallback.

### Nodes required

| Step | Node class | Config |
|------|-----------|--------|
| 1 | `FlowNode_Start` | — |
| 2 | `FlowNode_WaitForGameplayEvent` (base FlowGraph) | Tag: `AI.Alert.Threat` (define in your project's GameplayTags) |
| 3 | `FlowNode_GetBlackboardValues` | Key: `ThreatLevel`, Type: Int, output pin: `ThreatLevelOut` |
| 4 | `FlowNode_Branch` | Add-On: `FlowNodeAddOn_PredicateCompareBlackboardValue` |
| 5a | `FlowNode_Log` (placeholder for RunBehaviorTree) | Message: `"High threat — escalating to BehaviorTree"` |
| 5b | `FlowNode_Log` (placeholder for Idle) | Message: `"Low threat — returning to idle"` |
| 6 | `FlowNode_Finish` | — |

### Add-On configuration for step 4

Attach `FlowNodeAddOn_PredicateCompareBlackboardValue` to the `FlowNode_Branch` node:
- **Key**: `ThreatLevel`
- **CompareType**: `GreaterThan`
- **Value**: `5`

This add-on makes the branch evaluate `ThreatLevel > 5` directly against the subject actor's blackboard without extra Get/Compare nodes.

### ASCII flow diagram

```
[Start]
   |
   v
[WaitForGameplayEvent]  ← waits for tag "AI.Alert.Threat"
   |
   v
[GetBlackboardValues]  ← reads ThreatLevel from BB
   |
   v
[Branch]  ← add-on: PredicateCompareBlackboardValue(ThreatLevel > 5)
   |-- True  --> [Log: "High threat — escalating to BehaviorTree"] --> [Finish]
   |-- False --> [Log: "Low threat — returning to idle"]           --> [Finish]
```

### How to build it in the editor

1. **Create** `FA_AlertResponse` as a new `FlowAsset`.
2. **Add** `FlowNode_WaitForGameplayEvent` — set the gameplay tag to `AI.Alert.Threat`.
3. **Add** `FlowNode_GetBlackboardValues` — add key `ThreatLevel` (Int).
4. **Add** `FlowNode_Branch`.
5. **Right-click** the Branch node → **Add Add-On** → select `FlowNodeAddOn_PredicateCompareBlackboardValue`. Configure: Key=`ThreatLevel`, Compare=`GreaterThan`, Value=`5`.
6. Wire: Start → WaitForGameplayEvent → GetBlackboardValues → Branch.
7. Branch `True` → high-threat Log → Finish. Branch `False` → idle Log → Finish.

---

## Blackboard Integration Pattern

### Ensuring the blackboard exists

Before any read/write node runs, call `FlowNode_EnsureActorHasBlackboard` early in your flow (usually right after Start):

```
[Start] → [EnsureActorHasBlackboard] → [SetBlackboardValues ...] → ...
```

Skip this only when you are 100% certain the AI Controller has already initialized a blackboard (e.g., the flow is launched from a Behavior Tree that already owns one).

### Read vs Write nodes

| Goal | Node |
|------|------|
| Write initial or reset values | `FlowNode_SetBlackboardValues` |
| Write with in-place math (increment, clamp, etc.) | `FlowNode_SetBlackboardValuesV2` |
| Write to a *specific actor* (not the flow subject) | `FlowNode_SetBlackboardValuesOnActor` |
| Read values to drive later nodes | `FlowNode_GetBlackboardValues` |
| Evaluate a condition inline (no separate read) | `FlowNodeAddOn_PredicateCompareBlackboardValue` on a Branch/Gate node |

### Using predicates as add-ons

Add-ons in FlowGraph attach extra logic to a host node. `FlowNodeAddOn_PredicateCompareBlackboardValue` is the primary AI decision tool — it evaluates a blackboard key against a constant at the moment the host node fires, and the host node branches accordingly. This is preferable to a separate GetBlackboardValues → Compare chain because:
- It executes atomically (no frame gap between read and branch).
- It keeps the graph cleaner — one node instead of three.

### Wiring flows to Behavior Trees

AI flows are not replacements for Behavior Trees. The recommended pattern:

1. Behavior Tree runs the macro-level loop (patrol, investigate, attack).
2. When a significant event occurs, the BT service **starts an AI flow** via `StartFlowAsset` on the AI Controller.
3. The flow handles the event-driven micro-sequence (response animation, blackboard updates, one-shot logic) and **finishes**, returning control to the BT.

This keeps BT graphs clean and offloads scripted sequences to flows without losing the BT's reactive replanning capability.

### Best practices

- **Prefer `SetBlackboardValuesV2`** over chaining Get + compute + Set — it is safer and more readable.
- **Always guard reads with `EnsureActorHasBlackboard`** unless the flow's lifecycle guarantees a BB already exists.
- **Use gameplay tags for event names** — never hardcode string event names. Define them in `Config/DefaultGameplayTags.ini`.
- **Keep flows short** — if a flow exceeds ~10 nodes, split it into sub-flows connected via `FlowNode_SubGraph`.
- **Name BB keys consistently** — use PascalCase keys (`PatrolIndex`, `ThreatLevel`, `LastKnownTargetLocation`) matching the BT's blackboard asset.
