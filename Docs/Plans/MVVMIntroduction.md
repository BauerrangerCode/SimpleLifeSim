# Plan: MVC / MVVM Foundation (Epic E02)

Status: APPROVED by the user (all design questions answered; remaining items are engine-source verifications inside the steps). Epic E02.

## Goal
A robust, debuggable foundation for all later systems: replicated game data (Model) on the GameState, small server-side logic units (Controllers) run by one Orchestrator, and a UI path (ViewModel/View) that only reads the Model. Data is separated from logic and presentation.

## Decisions made in discussion
| # | Topic | Decision |
|---|---|---|
| D1 | Order-of-operations macro | **Separate plan afterwards** (CLAUDE.md 9.1 step 3). This plan only provides the *interim order* = order of the controller list. |
| D2 | Controller host | Orchestrator = `UWorldSubsystem` (server only). Controllers = plain `UObject`s created and owned by it. |
| D3 | Model structure | Sub-Models as **`UGameStateComponent`** subclasses on the GameState (replicated by engine standard). |
| D4 | Registration of Sub-Models | In the GameState constructor via `CreateDefaultSubobject`. Game module lists models; plugins only provide classes. |
| D5 | Local behavior (animation, particles, sound) | Pure **View**. Reads replicated state; one-shot effects via **GAS GameplayCues**. Orchestrator is server-only. No client-side controllers. |
| D6 | Per-player data | **Not decided here.** Architecture must not hard-code player count or per-player layout. |
| D7 | Change notification | **Both:** FieldNotify on Model components (for ViewModels) + own observer interface (for gameplay systems). Each delegate/mechanism justified in the plan. |
| D8 | Tests | Minimal tests in the foundation (Automation Spec: model, controller base, orchestrator order). |
| D9 | Write protection | **Convention + review only** (user decision). Mutators are named/marked as controller-only; checked with `review-against-guidelines`. Passkey token stays a possible later upgrade. Note: weaker than "robust" requirement; accepted trade-off. |
| D10 | Save/Load | Convention only: persistent model values carry `UPROPERTY(SaveGame)`. No save system in this plan. |
| D11 | Readiness | `GetModel()` may return null on clients; a one-time "model ready" signal from the GameState starts ViewModel initialization. ViewModels show placeholders until then. |
| D12 | Client -> server requests | Controllers cannot receive RPCs. **One Request-Component per domain** on the PlayerController (base class in core plugin) owns the Server RPCs, validates input, calls the Controller. |
| D13 | Controller list | `UDeveloperSettings` class with ordered `TArray<TSoftClassPtr<UControllerBase>>`. Order = interim init/tick order. |
| D14 | Controller-to-controller access | Via **interface**, delivered by the Orchestrator in `Initialize`. No casts to concrete classes. |
| D15 | Player count | Architecture is number-agnostic; 2 is the test case. CLAUDE.md "up to 2 players" wording to be adjusted after approval. |
| D16 | Module location | New **core plugin `KeyCore`** (module + folder `KeyCore`). Plugins and the game module depend on it. |
| D17 | Reference slice | **Time domain** end to end: TimeModel -> TimeController -> TimeViewModel -> simple widget showing day time. |
| D18 | ViewModel injection | **`UMVVMViewModelContextResolver`** (plugin-native). |
| D19 | Template code | All three Variant GameModes (all derive from `AGameModeBase`) switch to the new GameState; their old logic stays until migrated feature-by-feature later. |
| D20 | `GetModel()` | Returns a slim model container on the GameState with `Get<T>()` (e.g. `GetModel()->Get<UTimeModelComponent>()`), found via `FindComponentByClass`, then cached. New domains do not change `GetModel()`. |
| D21 | Gameplay observers | One **typed observer interface per Sub-Model** (e.g. `ITimeModelObserver`), explicit calls from the Model (visible in the call stack). Not needed in the Time slice (controller only writes); designed in the base but first used by a later feature. |
| D22 | Tick | One tick group; controllers tick in list order; tick is opt-in per controller. Phases left to the macro plan. |
| D24 | Time widget | Created by Claude via the MCP editor connection (ModelContextProtocol/AllToolsets). **Fallback if MCP cannot do it:** C++ base widget + German step-by-step guide, user creates the Blueprint. |
| D25 | Plugins enabled in `.uproject` | `ModelViewViewModel`, `ModularGameplay`. GameplayAbilities later (Combat plan). |
| D26 | Server target / net test | No server build target here (belongs to E01). Verification via PIE dedicated server + 2 clients. |
| D23 | Model-ready signal | Replicated `bModelReady` on the GameState set by the server after all Sub-Models exist; `OnRep_ModelReady` on the client verifies all Sub-Models are present, then calls one explicit registration point to start ViewModel initialization. |

## Additional requirements (agreed)
- Server-authoritative; only Controllers write the Model; clients read.
- Debuggability: one log category per system, console command printing Model state and controller order, clear errors on missing dependencies (fail loudly, `ensureMsgf`).
- Late join/reconnect: client receives full model state; UI tolerates not-yet-replicated state.
- No per-controller tick: the Orchestrator ticks controllers centrally (opt-in per controller).
- Names: clear, verb-based, Epic standard.

## Requirements and Acceptance Criteria (draft)
- [ ] `AGameState`-derived class holds Sub-Model components; `GetModel()` returns the model container/reference.
- [ ] A new Sub-Model is added by one class + one line in the GameState constructor.
- [ ] `UControllerBase` template exists; a derived controller works with `Initialize` / `Deinitialize` / optional `Tick`.
- [ ] Orchestrator creates controllers from the Developer Settings list, initializes them in list order, ticks them centrally, deinitializes in reverse order.
- [ ] Missing/invalid controller class or dependency produces a clear log + ensure, not silent failure.
- [ ] Time slice works in PIE with dedicated server + 2 clients: time advances on server, both clients show the same time, late-joining client sees the current time.
- [ ] Automation Specs for model, controller base and orchestrator order pass.
- [ ] Build succeeds without new warnings.

## Out of Scope
- Order-of-operations macro with compile-time validation (own plan).
- Per-player model (PlayerState) and player count decision.
- Save system, GAS content, any gameplay feature beyond the Time slice.
- Migrating Variant gameplay logic to the new architecture.
- Client-side controllers.

## Open Questions
None. Engine-source verifications to do *inside* the steps (not design questions):
- Step 3: GameState exists when `UWorldSubsystem::OnWorldBeginPlay` runs on the dedicated server; init-failure policy = stop all controllers, fail loudly (ensure + error log).
- Step 4: shared base interface for registering/unregistering Sub-Model observers.
- Step 6: exact `UMVVMViewModelContextResolver` virtuals; how the resolver finds the model for a given LocalPlayer in split-screen.
- Step 6: whether the MCP toolset can create and bind a Widget Blueprint (else fallback D24).

## Design (draft)
Layers and classes (all generic classes in core plugin; Time slice in game module unless decided otherwise):

| Class | Layer | Responsibility |
|---|---|---|
| `AGameStateBase` subclass (game module) | Model host | Creates Sub-Model components, `GetModel()`, fires model-ready signal |
| `UModelComponent` : `UGameStateComponent` (core) | Model | Base: FieldNotify support, replication setup, SaveGame convention, controller-only mutators |
| `UTimeModelComponent` (slice) | Model | Replicated day time |
| `UControllerBase` : `UObject` (core) | Controller | `Initialize(Context)`, `Deinitialize`, optional `Tick`, access to model via context, no own state |
| `UControllerOrchestrator` : `UWorldSubsystem` (core) | Controller | Server only. Creates/owns/inits/ticks/deinits controllers from settings |
| `UControllerSettings` : `UDeveloperSettings` (core) | Data | Ordered controller class list |
| `URequestComponent` : `UActorComponent` (core) | Bridge | Base for per-domain Server RPC components on the PlayerController |
| `UTimeController` (slice) | Controller | Advances time on server |
| `UTimeViewModel` : `UMVVMViewModelBase` (slice) | ViewModel | FieldNotify props from TimeModel |
| `UTimeViewModelResolver` (slice) | DI | Creates/injects the ViewModel |
| Time widget (slice) | View | Displays day time |

Data flow follows CLAUDE.md 8.2: View -> ViewModel -> RequestComponent (Server RPC) -> Controller -> Model -> replication -> ViewModel -> View.

Networking: Model replicated via component replication; Orchestrator and Controllers exist on the dedicated server only; ViewModels/Views are local; two-player test = PIE dedicated server + 2 clients.

## Implementation Steps (outline, to be detailed after open questions)
1. Enable plugins (D25); create `KeyCore` plugin skeleton and module; build.
2. `UModelComponent` + GameState with `GetModel()`; build + spec.
3. `UControllerBase`, settings, Orchestrator (init/tick/deinit, error handling); build + spec.
4. Observer interface + ready signal.
5. `URequestComponent` base.
6. Time slice: model, controller, request component (if needed), ViewModel, resolver.
7. Switch all four GameModes to the new GameState.
8. Console debug command + log categories.
9. 2-player PIE verification; update plan; report.

## Verification
Build output; Automation Specs; manual PIE (dedicated server + 2 clients, plus late-join client); review against guidelines; todo-report.

## Risks
- ModelViewViewModel plugin is Beta in 5.8.3.
- Write protection by convention only (D9).
- Replication timing of components on clients (D11).
- MCP-based widget creation is experimental (D24); fallback defined.
- Switching all variant GameModes (D19) can break the template maps; test each variant after the switch.
