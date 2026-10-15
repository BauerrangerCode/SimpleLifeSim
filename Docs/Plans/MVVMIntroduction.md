# Plan: MVC / MVVM Foundation (Epic E02)

Status: APPROVED by the user on 2026-10-15 (revised after review).
## Goal
A robust, debuggable foundation for all later systems: replicated game data (Model) on the GameState, small server-side logic units (Controllers) run by one Orchestrator, and a UI path (ViewModel/View) that only reads the Model. Data is separated from logic and presentation.

This plan builds the **read path** (Controller -> Model -> replication -> ViewModel -> View) end to end. The **request path** (View -> ViewModel -> Server RPC -> Controller) and **gameplay observers** are deliberately deferred (D12, D21) to a follow-up plan, so that nothing is built that the first slice does not use.

## Decisions
| # | Topic | Decision |
|---|---|---|
| D1 | Order-of-operations macro | **Separate plan afterwards** (CLAUDE.md 9.1 step 3). This plan only provides the *interim order* = order of the controller list. |
| D2 | Controller host | Orchestrator = `UTickableWorldSubsystem` (needed for central tick). Controllers = plain `UObject`s created and owned by it. "Server only" is decided in `OnWorldBeginPlay` by checking that an authority GameMode exists (not via NetMode in `ShouldCreateSubsystem`, which is unreliable on clients). On clients the subsystem exists but does nothing and does not tick. |
| D3 | Model structure | Sub-Models are **`UGameStateComponent`** subclasses (ModularGameplay) on the GameState. `UModelComponent` calls `SetIsReplicatedByDefault(true)` and implements `GetLifetimeReplicatedProps` explicitly. |
| D4 | Registration of Sub-Models | In the GameState constructor via `CreateDefaultSubobject`. Game module lists models; plugins only provide classes. |
| D5 | Local behavior (animation, particles, sound) | Pure **View**. Reads replicated state; one-shot effects via **GAS GameplayCues**. Orchestrator is server-only. No client-side controllers. |
| D6 | Per-player data | **Not decided here.** Architecture must not hard-code player count or per-player layout. |
| D7 | Change notification | **FieldNotify only** on Model components (for ViewModels). The typed gameplay observer interface is deferred (D21). |
| D8 | Tests | Minimal tests in the foundation (Automation Spec: model, controller base, orchestrator order). |
| D9 | Write protection | **Convention + review only** (user decision, no `friend`, no passkey). Mutators are named/marked as controller-only; checked with `review-against-guidelines`. Weaker than "robust"; accepted trade-off. |
| D10 | Save/Load | Convention only: persistent model values carry `UPROPERTY(SaveGame)`. No save system in this plan. |
| D11 | Readiness | `GetModel()` may return null on clients. ViewModels always get created by the resolver and show placeholders until the model is available. |
| D12 | Client -> server requests | **Deferred** to a follow-up plan ("Client Requests"). Design intent for then: one Request-Component per domain on the PlayerController owns the Server RPCs, validates input, calls the Controller. Which PlayerController class(es) carry them is decided there. |
| D13 | Controller list | `UDeveloperSettings` class with ordered `TArray<TSoftClassPtr<UControllerBase>>`. Order = interim init/tick order. |
| D14 | Controller-to-controller access | Via **interface**, delivered by the Orchestrator in `Initialize`. No casts to concrete classes. The Orchestrator verifies at init that every declared dependency was initialized **before** its dependent; wrong order or missing dependency = error log + `ensureMsgf`. |
| D15 | Player count | Architecture is number-agnostic; 2 is the test case. CLAUDE.md "up to 2 players" wording is adjusted in step 2. |
| D16 | Module location | New **core plugin `KeyCore`** (module + folder `KeyCore`). Plugins and the game module depend on it. `.uplugin` declares dependencies on `ModularGameplay` and `ModelViewViewModel`; Build.cs lists `ModelViewViewModel`, `FieldNotification`, `ModularGameplay`. |
| D17 | Reference slice | **Time domain** end to end, located in the **game module**: TimeModel -> TimeController -> TimeViewModel -> simple widget showing day time. |
| D18 | ViewModel injection | **`UMVVMViewModelContextResolver`** (plugin-native). |
| D19 | Template code | All **four** GameModes (`ASimpleLifeSimGameMode`, `ACombatGameMode`, `APlatformingGameMode`, `ASideScrollingGameMode`; all derive from `AGameModeBase`) switch to the new GameState. Old logic stays until migrated feature-by-feature later. |
| D20 | `GetModel()` | Returns a slim `UModelContainer` (non-replicated default subobject of the GameState, holds no data) with `Get<T>()`, e.g. `GetModel()->Get<UTimeModelComponent>()`. Resolves via `FindComponentByClass`, then caches. New domains do not change `GetModel()`. |
| D21 | Gameplay observers | **Deferred.** Typed observer interface per Sub-Model (explicit calls, visible in the call stack) is designed and built with the first gameplay system that needs it. |
| D22 | Tick | One tick group; controllers tick in list order; tick is opt-in per controller. Phases left to the macro plan. |
| D23 | Model-ready signal | **Uses the engine mechanism** `AGameStateBase::bReplicatedHasBegunPlay` / `OnRep_ReplicatedHasBegunPlay` / `HandleBeginPlay` instead of a custom flag. The GameState subclass overrides these (call `Super` first) and calls one explicit registration point afterwards. Sub-Models are default subobjects and exist on clients from construction; their initial values arrive with the GameState's initial replication. Visibility of the virtuals (`protected`?) is verified in step 5. |
| D24 | Time widget | Created by Claude via the MCP editor connection (ModelContextProtocol/AllToolsets). **Fallback if MCP cannot do it:** C++ base widget + German step-by-step guide, user creates the Blueprint. |
| D25 | Plugins enabled in `.uproject` | `ModelViewViewModel`, `ModularGameplay`. GameplayAbilities later (Combat plan). |
| D26 | Server target / net test | No server build target here (belongs to E01). Verification via PIE dedicated server + 2 clients. |
| D27 | Time replication | The Model stores a coarse **`MinuteOfDay`** (int32). The TimeController computes it from `GetServerWorldTimeSeconds()` and the configured scale and **writes only when the minute changes** (about once per in-game minute), so the value is not replicated every tick and the controller holds no state. Scale and start time live in a `UDeveloperSettings` class (`UTimeSettings`). The ViewModel needs no tick. |

## Delegate / mechanism justification (CLAUDE.md 7.5)
| Mechanism | Where | Justification |
|---|---|---|
| FieldNotify (`INotifyFieldValueChanged`) | `UModelComponent`, ViewModels | Engine mechanism of the MVVM plugin; View bindings are built on it. Explicitly allowed by CLAUDE.md 7.5. |
| `OnRep_...` functions | Model components, GameState | Replication callbacks are functions, allowed by CLAUDE.md 7.5. They trigger the FieldNotify broadcast on clients. |
| Central tick | Orchestrator (`UTickableWorldSubsystem`) | One tick on the server instead of one per controller; order is deterministic (list order) and visible in one call stack. Disabled on clients. Controller tick is opt-in. |
| Model-ready registration | GameState -> one explicit function call | No delegate; engine `OnRep_ReplicatedHasBegunPlay` override. |
| Gameplay observer interface | -- | Not in this plan (D21). |

## Additional requirements (agreed)
- Server-authoritative; only Controllers write the Model; clients read.
- Debuggability: one log category per system, console command printing Model state and controller order, clear errors on missing dependencies (fail loudly, `ensureMsgf`).
- Late join/reconnect: client receives full model state; UI tolerates not-yet-replicated state.
- No per-controller tick: the Orchestrator ticks controllers centrally (opt-in per controller).
- Names: clear, verb-based, Epic standard.

## Requirements and Acceptance Criteria
- [ ] `AGameStateBase`-derived class holds Sub-Model components; `GetModel()->Get<T>()` returns a Sub-Model.
- [ ] A new Sub-Model is added by one class + one line in the GameState constructor.
- [ ] `UControllerBase` exists; a derived controller works with `Initialize` / `Deinitialize` / optional `Tick`.
- [ ] Orchestrator runs only on the authority; it creates controllers from the Developer Settings list, initializes them in list order, ticks them centrally, deinitializes in reverse order. On clients it is inactive.
- [ ] Missing/invalid controller class, missing dependency, or dependency initialized after its dependent produces a clear error log + ensure, not silent failure.
- [ ] Model-ready registration is triggered through the engine `HasBegunPlay` path on server and clients.
- [ ] Time slice works in PIE with dedicated server + 2 clients: time advances on the server, `MinuteOfDay` replicates about once per in-game minute (not per tick), both clients show the same time, a late-joining client sees the current time. The widget shows a placeholder while the model is unavailable and does not crash.
- [ ] Console command prints Model state and controller order; each system has its own log category.
- [ ] Automation Specs for model, controller base and orchestrator order (incl. wrong-order detection) pass.
- [ ] CLAUDE.md is updated (D15, `KeyCore`, core-plugin dependency direction).
- [ ] Build succeeds without new warnings.

## Out of Scope
- Order-of-operations macro with compile-time validation (own plan).
- Request path (Request-Components, Server RPCs from UI) and gameplay observer interface -> follow-up plan.
- Per-player model (PlayerState) and player count decision.
- Save system, GAS content, any gameplay feature beyond the Time slice.
- Migrating Variant gameplay logic to the new architecture.
- Client-side controllers.
- Pausing / changing time speed at runtime.

## Open Questions
None. Engine-source verifications to do *inside* the steps (not design questions):
- Step 4: `OnWorldBeginPlay` runs after the GameState exists (spawn in `AGameModeBase`, `World.cpp` calls subsystems before `GameMode->StartPlay()`, confirmed by reading; full call order re-checked when implementing); authority detection via GameMode; init-failure policy = stop all controllers, fail loudly.
- Step 5: visibility/overridability of `OnRep_ReplicatedHasBegunPlay` and `HandleBeginPlay`.
- Step 6: exact `UMVVMViewModelContextResolver` virtuals (`CreateInstance`, `DestroyInstance` confirmed to exist); whether the MCP toolset can create and bind a Widget Blueprint (else fallback D24).
- Step 3: how `UModelComponent` implements `INotifyFieldValueChanged` and the UE_MVVM macros on a non-ViewModel class.

## Design
Layers and classes (generic classes in core plugin `KeyCore`; Time slice in the game module):

| Class | Layer | Responsibility |
|---|---|---|
| `AGameStateBase` subclass (game module) | Model host | Creates Sub-Model components and `UModelContainer`, `GetModel()`, overrides the HasBegunPlay path for the model-ready registration |
| `UModelContainer` : `UObject` (core) | Model access | Non-replicated; `Get<T>()` with cache; holds no data |
| `UModelComponent` : `UGameStateComponent` (core) | Model | Base: replication setup, FieldNotify support, SaveGame convention, controller-only mutators (convention) |
| `UTimeModelComponent` (slice) | Model | Replicated `MinuteOfDay` |
| `UControllerBase` : `UObject` (core) | Controller | `Initialize(Context)`, `Deinitialize`, optional `Tick`, model access via context, no own state |
| `UControllerOrchestrator` : `UTickableWorldSubsystem` (core) | Controller | Authority only. Creates/owns/inits/ticks/deinits controllers from settings; validates dependency order |
| `UControllerSettings` : `UDeveloperSettings` (core) | Data | Ordered controller class list |
| `UTimeSettings` : `UDeveloperSettings` (slice) | Data | Start minute, in-game minutes per real second |
| `UTimeController` (slice) | Controller | Computes `MinuteOfDay` from server time, writes on change |
| `UTimeViewModel` : `UMVVMViewModelBase` (slice) | ViewModel | FieldNotify props from TimeModel; weak pointer to the model; placeholder state |
| `UTimeViewModelResolver` (slice) | DI | Creates/injects the ViewModel, always returns an instance |
| Time widget (slice) | View | Displays day time |

Data flow (this plan): Controller -> Model (server) -> replication -> `OnRep` -> FieldNotify -> ViewModel -> View binding. The View -> ViewModel -> Controller direction from CLAUDE.md 8.2 is not exercised here (D12).

Networking: Model replicated via component replication; Orchestrator and Controllers act on the authority only; ViewModels/Views are local and never replicated; two-player test = PIE dedicated server + 2 clients.

## Implementation Steps
Each step builds before it is committed (CLAUDE.md 4.4); each step creates its log category together with its classes.
1. Enable plugins (D25); create `KeyCore` plugin skeleton and module (`.uplugin` + Build.cs dependencies per D16); build.
2. Update CLAUDE.md (D15 wording, `KeyCore` as core plugin, dependency direction, controller/model class names introduced here). Show the diff to the user.
3. `UModelComponent` (replication + FieldNotify) and `UModelContainer`; build + Spec.
4. `UControllerBase`, `UControllerSettings`, `UControllerOrchestrator` (authority check, init/tick/deinit, dependency-order validation, error handling); build + Spec.
5. Project GameState subclass: Sub-Model registration, `GetModel()`, HasBegunPlay-based model-ready registration. Switch all four GameModes to it (D19). Build; start each variant map once to check it still loads.
6. Time slice: `UTimeModelComponent`, `UTimeSettings`, `UTimeController` (+ dependency interface if needed), `UTimeViewModel`, `UTimeViewModelResolver`, widget (D24).
7. Console command (Model state + controller order).
8. 2-player PIE verification (dedicated server + 2 clients + late join); review against guidelines; todo-report; update plan with final state.

## Verification
Build output; Automation Specs; manual PIE (dedicated server + 2 clients, plus late-join client; check replication rate of `MinuteOfDay` with the console command or net stats); each variant map starts; review against guidelines; todo-report.

## Risks
- ModelViewViewModel plugin is Beta in 5.8.3.
- Write protection by convention only (D9).
- Widget may be constructed on a client before the GameState exists; mitigated by D11 (resolver always returns a ViewModel, placeholder state).
- FieldNotify on an `UActorComponent` is not a ready-made base class (unlike `UMVVMViewModelBase`); effort in step 3 may exceed expectation.
- MCP-based widget creation is experimental (D24); fallback defined.
- Switching all four GameModes (D19) can break the template maps; test each variant after the switch.
- The request path (CLAUDE.md 8.2, UI direction) stays unverified until the follow-up plan.
