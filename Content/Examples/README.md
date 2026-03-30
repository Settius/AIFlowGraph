# AIFlowGraph — Example Flows

> **Plugin**: AIFlowGraph · **Extends**: FlowGraph  
> **Purpose**: Adds AI blackboard integration and probability-based execution to Flow Assets.

---

## What AIFlowGraph Adds

AIFlowGraph extends the base FlowGraph plugin with:

| Feature | Description |
|---------|-------------|
| **Blackboard Read/Write** | Get and set values on any actor's `UBlackboardComponent` |
| **Blackboard Injection** | Ensure actors have a blackboard, injecting one if missing |
| **Weighted Random** | Execute one of N output pins based on configurable weights |
| **Guaranteed Roll** | Pseudo-random with increasing odds — guarantees success within N attempts |
| **Per-Actor Options** | Apply different blackboard entries to different actors in a single node |

All AI nodes inherit from `UAIFlowNode`, which provides:
- `GetBlackboardComponent()` — resolve the blackboard from the owning actor, controller, or game state
- `IFlowBlackboardInterface` — validate keys, query key types, gather keys by type
- Configurable `EActorBlackboardSearchRule` — search Actor only, Controller only, or Actor+Controller+GameState

---

## Node Reference

### Blackboard Nodes

| Node | Display Name | Description |
|------|-------------|-------------|
| `UFlowNode_GetBlackboardValues` | **Get Blackboard Values** | Reads blackboard keys and outputs them as data pins. Supports all value types (Bool, Int, Float, String, Vector, Rotator, Object, Enum, Class, Name). |
| `UFlowNode_SetBlackboardValues` | **Set Blackboard Values** | Writes configured entries to one or more actors' blackboards. Supports `SpecificActors` input pin or falls back to owning actor. |
| `UFlowNode_SetBlackboardValuesV2` | **Set Blackboard Values V2** | Enhanced version with per-actor option sets and assignment methods (InOrderWithWrapping, Random, etc.). |
| `UFlowNode_EnsureActorHasBlackboard` | **Ensure Actor Has Blackboard** | Checks if an actor has a blackboard; injects one if missing. Configurable injection rule (onto actor, controller, or skip). Outputs `Success` / `Failed`. |

### Probability Nodes

| Node | Display Name | Description |
|------|-------------|-------------|
| `UAIFlowNode_ExecutionRollWeighted` | **Roll Weighted** | Rolls one of N weighted output pins. Higher weight = higher chance. Zero-weight pins never fire. Uses `FRandomStream` for deterministic seeding. |
| `UAIFlowNode_ExecutionRollGuaranteed` | **Roll Guaranteed** | Pseudo-random with escalating odds. With `MaximumAttempts = 4`, odds are 25% → 50% → 75% → 100% on successive rolls. Has `Reset` input pin and `bResetOnSuccess` option. |

### Blackboard Value Types

The `FAIFlowBlackboardEntry` struct supports all standard Blackboard key types:

```
Bool, Int, Float, String, Name, Vector, Rotator, Object, Enum, Class
```

---

## Example 1: FA_SimplePatrol — Blackboard-Driven Patrol

**Goal**: Read a patrol index from a blackboard, branch on its value, then increment it.

### Flow Diagram

```
[Start] ──► [Get Blackboard Values] ──► [Switch (PatrolIndex)]
                                              │         │
                                        Pin "0"    Pin "1"
                                              │         │
                                              ▼         ▼
                                      [Move To A]  [Move To B]
                                              │         │
                                              └────┬────┘
                                                   ▼
                                       [Set Blackboard Values]
                                        (PatrolIndex = next)
                                                   │
                                                   ▼
                                              [Finish]
```

### Setup Steps

1. **Create the Flow Asset**  
   Content Browser → Add → Miscellaneous → **Flow Asset** → name it `FA_SimplePatrol`.

2. **Set up a Blackboard**  
   Create a `UBlackboardData` asset with an `Int` key named `PatrolIndex` (default = 0).

3. **Add Get Blackboard Values node**  
   - Add entry for key `PatrolIndex` (type: Int)
   - This outputs the value as a data pin

4. **Add Switch node** (from base FlowGraph)  
   - Connect input from Get Blackboard Values
   - Create output pins for `0` and `1`

5. **Add your game logic** per branch (e.g., custom FlowNodes that move the AI).

6. **Add Set Blackboard Values node**  
   - Entry: `PatrolIndex` = `(PatrolIndex + 1) % 2`
   - Or use two Set nodes, one per branch, with literal values

7. **Connect Finish** to loop or end the flow.

### Key Configuration

On the **Get Blackboard Values** node:
- `SpecificBlackboardAsset` → your Blackboard Data asset (optional if the owning actor already has one)
- `BlackboardSearchRule` → `ActorAndControllerAndGameState` (searches all three)

---

## Example 2: FA_AlertResponse — Weighted Random Reaction

**Goal**: When an AI detects a threat, randomly choose between three responses with weighted probabilities, with a guaranteed escalation mechanic.

### Flow Diagram

```
[Start] ──► [Ensure Actor Has Blackboard]
                     │            │
                  Success      Failed
                     │            │
                     ▼            ▼
             [Roll Weighted]   [Finish]
              │      │      │
          "Flee" "Alert" "Attack"
          (w:3)  (w:5)   (w:2)
              │      │      │
              ▼      ▼      ▼
          [Set BB] [Set BB] [Set BB]
          Fleeing  Alerting Attacking
              │      │      │
              └──────┼──────┘
                     ▼
            [Roll Guaranteed]
             MaxAttempts = 4
              │           │
         Guaranteed    Failure
              │           │
              ▼           ▼
         [Set BB:      [Finish]
          Enraged]
```

### Setup Steps

1. **Create Flow Asset** → `FA_AlertResponse`.

2. **Add Ensure Actor Has Blackboard**  
   - `SpecificBlackboardAsset` → your AI Blackboard (with keys: `AIState` (Name), `IsEnraged` (Bool))
   - `InjectRule` → `InjectOntoActorIfMissing`
   - Connect `Success` → Roll Weighted, `Failed` → Finish

3. **Add Roll Weighted node**  
   - `OutputPinOptions`:
     - `Flee`, Weight = 3 (30%)
     - `Alert`, Weight = 5 (50%)
     - `Attack`, Weight = 2 (20%)

4. **Set Blackboard Values** for each branch  
   - Flee branch: Set `AIState` = `"Fleeing"`
   - Alert branch: Set `AIState` = `"Alerting"`
   - Attack branch: Set `AIState` = `"Attacking"`

5. **Add Roll Guaranteed node**  
   - `MaximumAttempts` = 4 (25% base, escalating)
   - `bResetOnSuccess` = false
   - `GuaranteedOut` → Set `IsEnraged` = true
   - `FailureOut` → Finish

### Key Concepts

- **Roll Weighted** uses `FRandomStream` — set a seed for deterministic replays
- **Roll Guaranteed** with `MaximumAttempts = 4` means:
  - 1st trigger: 25% chance of Guaranteed output
  - 2nd trigger: 50% chance
  - 3rd trigger: 75% chance  
  - 4th trigger: 100% guaranteed
- **Ensure Actor Has Blackboard** safely injects a `UBlackboardComponent` if the actor doesn't have one, avoiding null reference crashes

---

## Example 3: FA_SquadSetup — Per-Actor Blackboard Configuration

**Goal**: Use the V2 Set node to assign different roles to squad members from a single node.

### Flow Diagram

```
[Start] ──► [Set Blackboard Values V2]  ──► [Finish]
                  │
          SpecificActors (input pin)
          EntriesForEveryActor: AlertLevel = 1
          PerActorOptions:
            [0] Role = "Leader"
            [1] Role = "Flanker"  
            [2] Role = "Support"
          AssignmentMethod: InOrderWithWrapping
```

### Setup Steps

1. **Create Flow Asset** → `FA_SquadSetup`.

2. **Add Set Blackboard Values V2 node**  
   - `EntriesForEveryActor` → add entry: `AlertLevel` (Int) = `1`
   - `PerActorOptions` → add 3 option sets:
     - Set 0: `Role` (Name) = `"Leader"`
     - Set 1: `Role` (Name) = `"Flanker"`
     - Set 2: `Role` (Name) = `"Support"`
   - `PerActorOptionsAssignmentMethod` → `InOrderWithWrapping`
   - Connect `SpecificActors` input pin to your squad actor array

3. **Result**: All actors get `AlertLevel = 1`. Actor 0 gets `Role = Leader`, Actor 1 gets `Role = Flanker`, Actor 2 gets `Role = Support`. If there's a 4th actor, it wraps to `Leader`.

---

## Creating a New Flow Asset with AI Nodes

1. **Content Browser** → Add → Miscellaneous → **Flow Asset**
2. Double-click to open the Flow Graph editor
3. Right-click → search for AI nodes:
   - `Get Blackboard Values`
   - `Set Blackboard Values` / `Set Blackboard Values V2`
   - `Ensure Actor Has Blackboard`
   - `Roll Weighted`
   - `Roll Guaranteed`
4. Configure the blackboard asset on each AI node (or let it inherit from the owning actor)
5. Wire nodes together using input/output pins
6. Assign the Flow Asset to an actor's `UFlowComponent`

### Blackboard Search Rules

| Rule | Searches |
|------|----------|
| `ActorOnly` | Only the actor's own blackboard |
| `ControllerOnly` | Only the actor's controller's blackboard |
| `GameStateOnly` | Only the game state's blackboard |
| `ActorAndControllerAndGameState` | All three, in order (default for most nodes) |

### Injection Rules

| Rule | Behavior |
|------|----------|
| `DoNotInjectIfMissing` | Fail silently if no blackboard found |
| `InjectOntoActorIfMissing` | Add a `UBlackboardComponent` to the actor |
| `InjectOntoControllerIfMissing` | Add a `UBlackboardComponent` to the controller |
