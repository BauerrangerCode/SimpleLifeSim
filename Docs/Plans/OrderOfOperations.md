# Plan: Order of Operations (Epic E03)

Status: APPROVED by the user on 2026-11-26. Story: E03-S01 (#19).

## Goal
Controllers declare what they need and in which order they run. The compiler computes the initialization order and the tick order and **aborts the build with a readable message** when the declarations are cyclic or impossible. A designer can still switch single Controllers off in Project Settings without code changes.

This replaces the interim solution of E02: the ordered `UControllerSettings.Controllers` list (D13), the runtime-only order check `ValidateDependencyOrder` (D14) and "tick order = list order" (D22).

## Decisions
| # | Topic | Decision |
|---|---|---|
| O1 | Where the Controllers are listed | **One central list in code**, written with a macro in the game module. Only the game module sees every Controller type (KeyCore, feature plugins, game; CLAUDE.md 9 dependency direction), so only there can the compiler check the whole graph. |
| O2 | Switching Controllers off | `UControllerSettings` keeps a **list of disabled Controllers** (Project Settings). The compiled list is the set of all Controllers; the settings only remove entries. |
| O3 | Relations a Controller declares | `DependsOn<X>`: needs access to X; X is delivered (as today through `ReceiveDependency`) and initialized before. `RunsAfter<X>`: order only, no access. `TicksAfter<X>`: tick order only. |
| O4 | Init order | Computed at compile time: stable topological sort over `DependsOn` + `RunsAfter`. Ties keep the position in the central list, so the result is deterministic and readable. Deinitialization runs in reverse. |
| O5 | Tick order | **One tick phase** as today (the Orchestrator ticks as `FTickableGameObject`, after `TG_PostPhysics`, `LevelTick.cpp`). Own declaration `TicksAfter<X>`; default is the init order. Stable topological sort over `TicksAfter`, ties keep the init order. Tick phases through Unreal tick groups are added only when a system needs them. |
| O6 | Error messages | C++20 (`CppStandardVersion.Default = Cpp20` in UBT) allows only literal `static_assert` messages. Each error kind has one fixed message, and it is raised inside a helper template named after the error kind whose template argument is the offending class, e.g. `TControllerOrderErrorDependencyCycle<UTimeController>`. The compiler prints that name in the error, so the class is visible. |
| O7 | `DependsOn` on a disabled Controller | **Start is aborted**: the Orchestrator starts no Controller, logs an error naming both classes and fires `ensureMsgf` (fail loudly, same policy as E02). `RunsAfter` / `TicksAfter` on a disabled Controller are ignored. In the editor, `UControllerSettings` additionally logs a warning when the disabled list is changed and breaks a `DependsOn`. |
| O9 | Controller missing from the central list | Cannot be detected at compile time: C++ cannot enumerate all subclasses, and a link-time trick fails for Controllers in plugins (each plugin DLL links before the game module exists). Checked **at start** instead: the Orchestrator compares `GetDerivedClasses(UControllerBase::StaticClass())` (`CoreUObject/Public/UObject/UObjectHash.h`) with the compiled list. Every native, non-abstract Controller class without `HideDropdown` (test classes) that is missing from the list **aborts the start** with error log + `ensureMsgf` naming the class. An entry in `DisabledControllers` that cannot be resolved to a class logs a warning. An Automation Spec runs the same comparison. The reverse direction (list entry without a class) does not compile. |
| O8 | Proof that bad declarations break the build | The check logic is `constexpr`. Test graphs with and without errors are checked with `static_assert` in every build. In addition, one manual run: a real cycle is added, the build fails with the message, the output is recorded in this plan, the change is reverted. |

## Why not an existing Unreal system (CLAUDE.md 3, Unreal technology first)
- `FSubsystemCollectionBase::InitializeDependency` (`Engine/Public/Subsystems/SubsystemCollection.h`) orders subsystem initialization, but at runtime only and only for subsystems; Controllers are plain UObjects (MVVM plan D2).
- `FTickFunction::AddPrerequisite` (`Engine/Classes/Engine/EngineBaseTypes.h`) orders tick functions at runtime. It needs one tick function per Controller and cannot detect cycles at compile time.
- Neither meets the requirement "cycles abort the build". The compile-time part is therefore our own; at runtime the existing Orchestrator (a `UTickableWorldSubsystem`) stays.

## Requirements and Acceptance Criteria
- [ ] A Controller declares `DependsOn`, `RunsAfter` and `TicksAfter` with one macro in its class body.
- [ ] The game module lists all Controllers with one macro; the compiler computes init order and tick order from it.
- [ ] Each of these aborts the build with its fixed message and the offending class in the error: init cycle, tick cycle, relation to a class missing from the central list, relation to itself, a class listed twice, a listed type that is not a `UControllerBase`.
- [ ] `static_assert` tests over test graphs cover every error kind and the computed orders, and run in every build.
- [ ] One manual build with a real cycle fails with a readable message; the output is recorded in this plan.
- [ ] The Orchestrator initializes in the computed init order, ticks in the computed tick order, deinitializes in reverse init order, and delivers `DependsOn` Controllers through `ReceiveDependency`.
- [ ] Disabled Controllers (Project Settings) are not created. A `DependsOn` on a disabled Controller aborts the start with error log + ensure; `RunsAfter`/`TicksAfter` on it are ignored. Changing the disabled list in the editor warns about broken `DependsOn`.
- [ ] A native, non-abstract Controller class missing from the central list aborts the start with error log + ensure naming the class (test classes with `HideDropdown` are skipped); an unresolvable `DisabledControllers` entry logs a warning; an Automation Spec checks that every Controller class of the project is listed.
- [ ] `UTimeController` uses the macro; `UControllerSettings.Controllers`, `ValidateDependencyOrder` and the `DefaultGame.ini` entry are removed.
- [ ] `KeyCore.PrintState` prints the init order, the tick order and the disabled Controllers.
- [ ] Automation Specs for the runtime part pass (order building, Orchestrator order, disabled handling).
- [ ] Build succeeds without new warnings.
- [ ] Two-player check (story E03-S05): PIE with dedicated server + 2 clients, including the E02 checks that were deferred until after E03.

## Out of Scope
- Tick phases over several Unreal tick groups (O5).
- Client-side Controllers (MVVM plan D5).
- Ordering of anything other than Controllers (subsystems, actors, components).
- Request path and gameplay observers (MVVM plan D12, D21).
- Enabling/disabling Controllers at runtime (only at start).

## Open Questions
None. Technical checks done *inside* the steps (not design questions):
- Step 1: UHT accepts a custom macro in a `UCLASS` body that expands to a type alias and a `virtual` function override (no `UPROPERTY`/`UFUNCTION` inside). Fallback if not: the declaration moves next to the class as a template specialization `TControllerOrderDeclaration<UMyController>` in the same header.
- Step 1: whether `TStaticArray` is usable in the needed `constexpr` contexts (constructors are `constexpr`); otherwise plain fixed-size arrays inside `constexpr` structs.
- Step 1: which compiler output MSVC produces for the helper templates (O6); adjust naming if the class does not appear in the first error line.

## Design

### Placement and dependency direction
- Generic part in **`KeyCore`** (new header `Controller/KeyCoreControllerOrder.h`; only Epic types, no feature plugin or game dependency).
- The central list in the **game module** (`Source/SimpleLifeSim/Controllers/SimpleLifeSimControllerOrder.h/.cpp`). It includes every Controller header; dependency direction game -> plugins -> KeyCore is kept.

### Classes and types
| Type | Layer | Responsibility |
|---|---|---|
| `KEYCORE_CONTROLLER_ORDER(...)` macro (KeyCore) | Controller (declaration) | Placed in a Controller's class body. Expands to a type alias with the declared relations and to the `GetDependencies()` override, so the runtime dependency list can no longer drift from the declaration. |
| `TDependsOn<T>`, `TRunsAfter<T>`, `TTicksAfter<T>` (KeyCore) | Controller (declaration) | Empty tag templates that name one relation. |
| `TControllerOrderSolver<ControllerTypes...>` (KeyCore) | Controller (compile time) | `constexpr` validation and stable topological sorts (O4, O5). Raises the error templates (O6). |
| `TControllerOrderError<Kind><T>` templates (KeyCore) | Controller (compile time) | One template per error kind with a fixed `static_assert` message; `T` is the offending class. |
| `KEYCORE_DEFINE_CONTROLLER_ORDER(FunctionName, ControllerTypes...)` macro (KeyCore) | Controller (declaration) | Used once in the game module. Instantiates the solver and defines a function that returns the runtime `FControllerOrder`. |
| `FControllerOrder` (KeyCore, plain struct, not a `USTRUCT`) | Controller (runtime data) | Result of the solver at runtime: Controller classes in init order, tick order as indices. Built once from `StaticClass()` of each type. Pure data, no rules. |
| `UControllerOrchestrator` (KeyCore, changed) | Controller | `StartControllers(const FControllerOrder&)`: checks for unlisted Controller classes (O9), filters disabled Controllers, checks O7, creates, delivers dependencies, initializes, ticks, deinitializes. `ValidateDependencyOrder` and its helpers are removed. |
| `UControllerBase` (KeyCore, changed) | Controller | `GetDependencies()` stays virtual but is only implemented by the macro (convention: never overridden by hand; checked in review). |
| `UControllerSettings` (KeyCore, changed) | Data | `Controllers` is replaced by `DisabledControllers` (`TArray<TSoftClassPtr<UControllerBase>>`, `Config`, `EditAnywhere`). Editor-only `PostEditChangeProperty` warns about a broken `DependsOn` (O7); it reads `GetDependencies()` of all non-abstract Controller class defaults. |
| `ASimpleLifeSimGameState` (game, changed) | Model host | Passes the game's `FControllerOrder` to `StartControllers` at the model-ready point (E02 D23). |
| `UTimeController` (game, changed) | Controller | Uses `KEYCORE_CONTROLLER_ORDER()` (no relations today). |
| `KeyCore.PrintState` (KeyCore, changed) | Debug | Prints init order, tick order and disabled Controllers. |

Names are proposals; they are part of this plan's approval.

### Example (illustrative, final syntax fixed in step 1)
```cpp
// In a Controller header
UCLASS()
class UWeatherController : public UControllerBase
{
	GENERATED_BODY()
	KEYCORE_CONTROLLER_ORDER(TDependsOn<UTimeController>, TTicksAfter<UTimeController>)
	...
};

// Game module, once
KEYCORE_DEFINE_CONTROLLER_ORDER(GetSimpleLifeSimControllerOrder, UTimeController, UWeatherController)
```

### Error kinds (each one fixed message, O6)
Init cycle (over `DependsOn` + `RunsAfter`), tick cycle (over `TicksAfter`), relation to a class missing from the central list, relation to itself, class listed twice, listed type not derived from `UControllerBase`.

### Data / editor exposure
- `UControllerSettings.DisabledControllers` in Project Settings ("KeyCore Controllers").
- No other values; the order is code by design (O1). No DataAssets.

### Networking
Unchanged from E02: the Orchestrator and Controllers run only on the authority; the Model replicates; ViewModels and Views stay local. The order system has no network traffic.

### Delegates
None. The GameState calls the Orchestrator directly; the settings warning is a `PostEditChangeProperty` override, not a delegate.

### Standard library use
Only `<type_traits>` style compile-time helpers where Unreal has no equivalent (Unreal's `TIsDerivedFrom` is used for the base-class check). No STL containers (CLAUDE.md 7.2).

## Implementation Steps
Each step builds before it is committed (CLAUDE.md 4.4); each step reports warnings and errors.
1. KeyCore: tag templates, error templates, `TControllerOrderSolver` (validation + both sorts), `KEYCORE_CONTROLLER_ORDER`, `KEYCORE_DEFINE_CONTROLLER_ORDER`, `FControllerOrder`. Verify the three technical checks from "Open Questions". `static_assert` tests over test graphs for every error kind and for both orders. Build.
2. Manual negative build: add a real cycle, build, record the compiler output in this plan, revert. (No commit of the cycle.)
3. Orchestrator: `StartControllers(const FControllerOrder&)`, tick order, unlisted-class check (O9), disabled filtering and O7 check; remove `ValidateDependencyOrder` and its specs; new Automation Specs for order building, Orchestrator order and disabled handling. Build + specs.
4. Settings: replace `Controllers` with `DisabledControllers`, editor warning, remove the `DefaultGame.ini` entry. Build.
5. Game module: central list with `UTimeController` (function declared in a game module header, macro comment updated), `UTimeController` uses the macro, GameState passes the order. Build; start each variant map once headless and check that the Controller starts.
6. `KeyCore.PrintState` shows init order, tick order and disabled Controllers. Update CLAUDE.md 9.0 (classes) and the `UControllerSettings` doc comment. Build.
7. Story E03-S05: two-player PIE (dedicated server + 2 clients, late join, widget placeholder), review against guidelines, todo-report, update this plan with the final state.

## Progress Notes
### Step 1 (2026-11-26)
- UHT accepts `KEYCORE_CONTROLLER_ORDER` in a `UCLASS` body (checked with `UKeyCoreTestControllerB`, whose hand-written `GetDependencies()` the macro replaced; the existing specs still pass).
- Deviation: the generic part is split into two headers. `KeyCoreControllerOrder.h` (tags, declaration, macro) is light and included by `KeyCoreControllerBase.h`; `KeyCoreControllerOrderSolver.h` (solver, error templates, `FControllerOrder`, `KEYCORE_DEFINE_CONTROLLER_ORDER`) needs the complete `UControllerBase` and is only included where the central list is defined.
- The solver uses plain fixed-size arrays inside `constexpr` structs; `TStaticArray` was not needed.
- `static_assert(sizeof(FResult) > 0)` in the solver avoids MSVC warning C4686 for the first use of a result type in a constant expression.

### Step 2: manual negative build (2026-11-26)
Two broken lists were added temporarily to `KeyCoreControllerOrderStaticTests.cpp` and reverted afterwards.

The first run showed that MSVC instantiates alias templates inside discarded `if constexpr` branches, which produced follow-up errors (`TNthTypeFromParameterPack<-1>`). The error dispatch was changed to partial specializations (`TProblemReport`), so only the matching error template is instantiated.

Init cycle, `KEYCORE_DEFINE_CONTROLLER_ORDER(GetBrokenCycleOrder, FCycleFirst, FCycleSecond)` (MSVC output, German compiler locale, shortened paths):
```
KeyCoreControllerOrderSolver.h(97,61): error C2338: static_assert failed: 'KeyCore controller order: the DependsOn / RunsAfter declarations form a cycle. The template argument of TControllerOrderErrorInitOrderCycle is part of the cycle.'
KeyCoreControllerOrderSolver.h(255,99): note: Siehe Verweis auf die gerade kompilierte Klasse Vorlage-Instanziierung "TControllerOrderErrorInitOrderCycle<KeyCoreControllerOrderStaticTests::FCycleFirst>".
KeyCoreControllerOrderSolver.h(193,25): error C2338: static_assert failed: 'KeyCore controller order is invalid. The error above names the class.'
```

Relation to an unlisted class, `KEYCORE_DEFINE_CONTROLLER_ORDER(GetBrokenUnlistedOrder, FControllerB)`:
```
KeyCoreControllerOrderSolver.h(90,61): error C2338: static_assert failed: 'KeyCore controller order: a Controller declares a relation to a class that is missing from KEYCORE_DEFINE_CONTROLLER_ORDER. The template arguments of TControllerOrderErrorRelationToUnlisted are the Controller and the missing class.'
KeyCoreControllerOrderSolver.h(249,5): note: Siehe Verweis auf die gerade kompilierte Klasse Vorlage-Instanziierung "TControllerOrderErrorRelationToUnlisted<KeyCoreControllerOrderStaticTests::FControllerB,KeyCoreControllerOrderStaticTests::FControllerA>".
KeyCoreControllerOrderSolver.h(193,25): error C2338: static_assert failed: 'KeyCore controller order is invalid. The error above names the class.'
```

Observation: with both broken lists in one file, MSVC reported only the first. This does not matter in practice, because there is exactly one central list (O1). The class names appear in the "note" line right after the message, not in the error line itself (risk from the plan confirmed, acceptable).

### Review fixes after steps 1 and 2 (approved by the user on 2026-11-26)
No design change.
- Type traits: only `std::` traits (`std::is_same_v`, `std::is_base_of_v`); `TIsDerivedFrom` is no longer used. The engine's Core module itself uses `std::is_base_of_v` and has no `TIsSame` any more.
- `BuildOrder()` gets classes through `KeyCore::Private::GetControllerClass<T>()`, which returns null for types that are no Controller. A list with such a type now fails with the `NotAController` message only (checked with a temporary negative build, reverted), without a follow-up "StaticClass is not a member" error.
- Test Controllers in `KeyCoreControllerOrderStaticTests.cpp` carry the `U` prefix, because they derive from `UObject` (Epic standard), although they have no `UCLASS`.
- The order test helpers are one function `HasOrder<SolverType, Member>` with a member pointer to `InitOrder` or `TickOrder`; `HasInitOrder` / `HasTickOrder` only forward to it.
- Step 5 additionally declares the function created by `KEYCORE_DEFINE_CONTROLLER_ORDER` in a game module header (needed by the GameState anyway; avoids a missing-prototype warning on Clang), and the macro comment says so.

## Verification
| Criterion | Check |
|---|---|
| Errors abort the build | `static_assert` tests in every build (step 1) + one manual cycle build with recorded output (step 2) |
| Computed orders | `static_assert` tests (step 1); Automation Spec on `FControllerOrder` (step 3) |
| Orchestrator order, disabled handling, O7 | Automation Specs (step 3) |
| Unlisted Controller classes (O9) | Automation Spec: every project Controller class is listed (step 3); manual: remove `UTimeController` from the list once, start fails with its name in the log, revert (step 5) |
| Editor warning | Manual in the editor: disable a Controller that another depends on, check the Output Log (step 4) |
| Time slice still works | Headless start of each variant map (step 5); two-player PIE (step 7) |
| Console output | `KeyCore.PrintState` in PIE (step 6) |
| No new warnings | Build output after each step |

## Risks
- Template error output differs between compilers; MSVC may print the helper template name only in a later line of the error. Mitigation: checked in step 1, naming adjusted.
- UHT may reject the macro in the class body; fallback defined (Open Questions, step 1).
- Compile time grows with the number of Controllers (`constexpr` sorting is O(n^2)). Expected to be negligible for dozens of Controllers; measured once in step 5.
- The central list is a single place every new Controller must be added to. Forgetting it is caught at start and by the Automation Spec (O9), not at compile time.
- O9 relies on all Controller modules being loaded when the Orchestrator starts. Modules loaded later (e.g. a plugin loaded on demand) are not seen. Today every Controller module loads at startup.
- `GetDependencies()` could be overridden by hand and drift from the declaration; mitigated by convention + review (same approach as E02 D9).
