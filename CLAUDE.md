# CLAUDE.md - SimpleLifeSim

This file defines how Claude works in this project. It is binding for every session.
If an instruction here conflicts with a request in chat, ask before acting.

---

## 1. Project Overview

- **Engine:** Unreal Engine 5.8.3, source build; the engine root is the parent folder of the project root (`..`).
- **Project root:** the repository root (the folder that contains `SimpleLifeSim.uproject`). All paths in project files and docs are relative to it.
- **Game module:** `SimpleLifeSim` (Runtime). Targets: `SimpleLifeSim`, `SimpleLifeSimEditor`.
- **Enabled plugins (from .uproject):** StateTree, GameplayStateTree, ModelingToolsEditorMode, ModelContextProtocol (Experimental), AllToolsets, ModelViewViewModel (Beta), ModularGameplay, KeyCore (project plugin, see 9.0).
- **Goal:** A fully playable RPG for **one or two players (co-op multiplayer)** with:
  - playable character(s),
  - fifteen story characters that drive the narrative,
  - multiple enemy types,
  - five boss fights with multiple phases each.
- **Current state:** Based on the UE Third Person template including the `Variant_Combat`, `Variant_Platforming` and `Variant_SideScrolling` sample code. **This code is kept and refactored into the game.** Reuse existing code where it fits the architecture; do not rewrite working code without a reason stated in the plan.

---

## 2. Communication and Language

- **Chat with the user: German.**
- **Everything else: English.** This includes code, identifiers, comments, log messages, commit messages, plan files, documentation, config comments and this file.
- Be concise. No filler, no marketing language.
- The user is learning MVVM. When MVVM-related decisions are made, explain the reasoning in chat (German) so the user can follow and learn.

---

## 3. Honesty and No Assumptions

- Stick to facts. Never invent APIs, engine behavior, file contents, or test results.
- If something is unknown, unverifiable, or cannot be named, say so explicitly.
- **Never make assumptions about intent or requirements. Ask the user what is meant.** This applies to gameplay design, naming, architecture, scope and edge cases.
- **Unreal technology first:** Always prefer existing Unreal Engine systems and plugins over custom solutions or third-party tools. A custom solution requires a justification in the plan explaining why the Unreal option does not fit.
- Verify engine APIs against the engine source in this repository (`Engine/Source`, `Engine/Plugins`) instead of relying on memory. UE APIs change between versions; this project uses 5.8.3.
- **Missing material or unclear tasks are marked in code or docs with:**
  ```
  //TODO claude: <description of the missing material or the unclear task>
  ```
  - The format is exact: `//TODO claude: ` followed by the description.
  - Every time such a marker is added, tell the user in chat (file + line + short reason).
  - Never leave a placeholder implementation without this marker.
- If a task was not fully completed, a build was not run, or a test was not executed, say so clearly. Never report unverified work as done.

---

## 4. Workflow: No Plan, No Execution

Every feature, refactoring, or non-trivial change follows these steps. **No code is written before step 3 is complete.**

1. **Clarify** - Ask questions until requirements, scope, and acceptance criteria are unambiguous.
2. **Plan** - Write a plan file in `SimpleLifeSim/Docs/Plans/<FeatureName>.md` (template below).
3. **Approval** - The user explicitly approves the plan. Without approval, nothing is implemented.
4. **Implement** - In small, reviewable steps that follow the plan. If the plan turns out to be wrong or incomplete, stop, update the plan, and get approval again.
   - **Commit after every completed step** as a fallback point (local commit, following the commit convention in section 13). A step is completed when it builds; if it does not build, it is not committed. Never bundle several steps into one commit. If a step fails, return to the last commit instead of patching on top of a broken state - after telling the user.
   - Commits need no confirmation; pushing does (section 5).
5. **Verify** - Build, run tests, and check the result against the acceptance criteria.
6. **Report** - What was done, what was verified and how, what is open (including all new `//TODO claude:` markers).

Trivial changes (typo, comment fix) may skip the plan file, but still require the user's go-ahead.

### Plan template

```markdown
# Plan: <Feature name>

## Goal
What the feature does, from the player's / designer's perspective.

## Requirements and Acceptance Criteria
- [ ] Measurable criterion 1
- [ ] ...

## Out of Scope
What is explicitly not part of this plan.

## Open Questions
Everything that is still unclear. Must be empty before approval.

## Design
- Classes / components / interfaces (new and changed), with responsibilities.
- MVVM layer assignment for every new class (Model / ViewModel / View).
- Data: which values are exposed to the editor, which DataAssets are used.
- Networking: what is replicated, what runs on server / owning client / all clients.
- Dependencies on other systems, modules and plugins.

## Implementation Steps
1. ...

## Verification
How each acceptance criterion is checked (build, automation test, manual test in editor, 2-player PIE test).

## Risks
```

---

## 5. Human-in-the-Loop

Configured in `.claude/settings.json`:
- **Without confirmation:** `Read`, `Edit`, `Write`, and `Bash` in general (builds, tests, analysis scripts).
- **With confirmation:** pushing to version control, installing software/packages, and deleting files.

Rules:
- Work in small steps. After each step, report briefly what was done and the result (including warnings and errors), so the user can review single steps.
- Never run destructive commands without explicit approval for that specific command.
- **Never modify engine code** (`..\Engine`). The engine is read-only reference material.

---

## 6. Context Management

Claude tells the user (in German) when it makes sense to:
- run **`/compact`** - e.g. after a feature step is finished, after long build logs or large file reads, or when earlier parts of the conversation are no longer needed;
- **start a new chat** - e.g. when switching to an unrelated feature, or when the conversation has become long enough that earlier instructions or decisions risk being lost.

Before suggesting either, make sure the current state is persisted (plan file updated, open TODOs listed), so nothing gets lost.
Note: Claude can only estimate its context usage; the suggestion is a judgment, not an exact measurement.

---

## 7. Coding Standards

Binding references:
- Epic C++ Coding Standard: https://dev.epicgames.com/documentation/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
- Clean Code summary: https://gist.github.com/wojteklu/73c6914cc446146b8b533c0988cf8d29

**Precedence:** Epic's standard governs naming and formatting. Clean Code governs design, function structure, and readability. **In case of a real conflict, Epic's standard wins.**

### 7.1 File Header

Every new source file (`.h`, `.cpp`, `.cs`) starts with:
```cpp
// Copyright Torben Borghoff, Keygen-Games
```
The same notice is set in Project Settings (`DefaultGame.ini` -> `CopyrightNotice`) so the editor's class wizard uses it.

Files that originate from Epic (template code) **keep the Epic notice**. Our notice is added as an addendum in the line below:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Torben Borghoff, Keygen-Games
```

### 7.2 Epic Coding Standard (key rules)

- **Type prefixes:** `U` (UObject), `A` (AActor), `S` (SWidget), `I` (interface), `F` (struct / other), `E` (enum), `T` (template), `b` (bool variables, e.g. `bIsDead`).
- **PascalCase** for types, functions, variables. No underscores in names except where Epic conventions require them.
- Function names are verbs (`ApplyDamage`). Bool-returning functions ask a question (`IsAlive`, `CanAttack`).
- **Braces on their own line** (Allman style). Always use braces, even for single-line blocks.
- **Tabs for indentation**, 4 spaces wide.
- `enum class` with `UENUM(BlueprintType)`; use `uint8` as the underlying type when exposed to Blueprint.
- Use `nullptr`, `override`, `final` where appropriate. Use `const` wherever possible (parameters, methods, locals).
- `auto` only where Epic allows it (lambdas, iterators, when the type is obvious from the right-hand side).
- Use Unreal containers and types (`TArray`, `TMap`, `TSet`, `FString`, `FName`, `FText`, `int32`). No STL containers in gameplay code.
- `TObjectPtr<>` for UObject member pointers; every UObject member pointer is a `UPROPERTY()` so the GC tracks it. `TWeakObjectPtr<>` for non-owning references that must not keep objects alive.
- Use `TEXT()` for string literals.
- Include-what-you-use; forward-declare in headers where possible; `.generated.h` is the last include in a header.
- Every UPROPERTY / UFUNCTION / class gets a `/** ... */` doc comment that explains **why**, not what.

### 7.3 Clean Code (key rules)

- **Names:** descriptive, unambiguous, pronounceable, searchable. No abbreviations, no encodings, no magic numbers.
- **Functions:** small, do one thing, one level of abstraction, few arguments (prefer a struct for more than three), no flag arguments, no hidden side effects.
- **Comments:** explain intent and consequences. Never comment out code; delete it. No redundant or noise comments.
- **Design:** single responsibility, prefer composition (components) over inheritance, prefer polymorphism / interfaces over `if`/`switch` on types, follow the Law of Demeter, keep configurable data at high levels.
- **Structure:** related code close together, vertical ordering top-down, dependent functions close to each other.
- **Error handling:** fail loudly and early (see 7.5); do not silently swallow errors.
- **Code smells to avoid:** rigidity, fragility, needless complexity, needless repetition, opacity.
- **Boy Scout Rule:** leave touched code cleaner than found - but only within the approved plan's scope; other improvements are suggested, not done.

### 7.4 No Hardcoded Values

Gameplay code contains **no fixed values** (numbers, strings, asset paths, tags). Use, in this order:

1. **Editor-exposed properties** on the owning class:
   ```cpp
   /** Time in seconds the enemy waits before it attacks again. */
   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (ClampMin = "0.0", Units = "s"))
   float AttackCooldown = 1.5f;
   ```
   - Default to `EditDefaultsOnly` + `BlueprintReadOnly`. Use `EditAnywhere` only when per-instance tuning in a level is required. Use `BlueprintReadWrite` only when Blueprints must change the value at runtime.
   - Always set `Category` (use `|` for sub-categories) and meaningful `ClampMin`/`ClampMax`/`Units` meta where they apply.
   - The C++ default value is only a sensible starting point; the authoritative value lives in the editor.
2. **DataAssets** for data shared between multiple actors (character stats, enemy archetypes, boss phases, dialogue).
3. **`UDeveloperSettings`** for project-wide settings (appear in Project Settings).
4. **Gameplay Tags** instead of string or `FName` literals for identifiers, states and categories.
5. **Asset references** via `TSoftObjectPtr` / `TSoftClassPtr` / `TSubclassOf` exposed as properties. Never hardcode asset paths (no `ConstructorHelpers::FObjectFinder` with literal paths).

Allowed literals: `0`, `1`, `-1`, `nullptr`, `true`/`false`, and values that are mathematically fixed (e.g. `0.5f` for halving). If unsure whether a literal is allowed, ask.

Player-facing text is always `FText` (localizable), never `FString` literals.

### 7.5 Unreal-Specific Practices

- **Logging:** one log category per system (`DECLARE_LOG_CATEGORY_EXTERN`), `UE_LOG` with appropriate verbosity. No `GEngine->AddOnScreenDebugMessage` in committed code unless wrapped in a debug-only path.
- **Assertions:** `check()` for programmer errors that must never happen, `ensure()`/`ensureMsgf()` for recoverable unexpected states, plain `if` + log for expected runtime conditions (e.g. missing optional data).
- **Tick:** disabled by default (`PrimaryActorTick.bCanEverTick = false`). Prefer events, delegates and timers. Enabling Tick requires a justification in the plan.
- **Communication between systems:** interfaces instead of hard casts to concrete classes.
- **Avoid delegates where a direct call works** (delegates are harder to debug). Prefer explicit calls through interfaces, so the call stack shows who calls whom. Replication callbacks (`OnRep_...`) are functions and are allowed.
  - Existing engine delegates and delegate-based engine mechanisms (e.g. MVVM `FieldNotify`, GAS attribute change notifications, UMG button events like `OnClicked`) are allowed where Unreal intends them to be used.
  - Every delegate in our own code needs a short justification in the plan (e.g. "button press: UMG is designed around `OnClicked`").
- **Blueprints:** logic lives in C++. Blueprints are thin subclasses for assets, tuning, and visual scripting only where the plan says so.

### 7.6 Networking (Multiplayer, co-op)

- **Player count:** the architecture does not hard-code the number of players (D15 in the MVVM plan). Two players is the tested and designed case.
- **Mode:** co-op, offered as **two game modes that share the same code path**:
  - **Online:** players connect to a dedicated server.
  - **Local split-screen:** the game starts a **local dedicated server in the background**, and both local players connect to it.
  - Consequence: there is exactly one network architecture (client / dedicated server). Gameplay code never has a separate "offline" path. A `Server` build target is required.
- All gameplay code must be written **multiplayer-aware from the start**. Retrofitting replication is not acceptable.
- The **server is authoritative** for gameplay state. Clients request actions; the server validates and executes.
- Use GAS replication (abilities, effects, attributes) instead of custom RPCs where GAS covers the case.
- Mark replicated properties with `Replicated` / `ReplicatedUsing=OnRep_...` and register them in `GetLifetimeReplicatedProps`. Use the narrowest replication condition that works.
- RPC naming: `Server...`, `Client...`, `Multicast...`. Server RPCs validate their input.
- Check authority / local control explicitly (`HasAuthority()`, `IsLocallyControlled()`), never implicitly.
- **ViewModels and Views are local** to each player and are never replicated. They observe replicated Model state.
- Every feature plan states how it is tested with two players (PIE with 2 clients).


---

## 8. Architecture Paradigm: Model / Controller / Presentation (MVC + MVVM)

The base paradigm separates **game data** from **game logic** and from **presentation**, throughout the game - not only in the UI.
For the UI, this is implemented with MVVM using Epic's **UMG Viewmodel** plugin (`ModelViewViewModel`, marked **Beta** in UE 5.8.3).
Actors are used where a connection to the game world is required (e.g. enemy/NPC spawners, characters, interactables).

### 8.1 Layers and Responsibilities

| Layer | Contains | Responsibilities | Must NOT |
|---|---|---|---|
| **Model** | All game-relevant data: inventory, health, experience, quests, world state, NPC locations, completed dialogues, time of day, ... | Holds data. Only trivial validation and read access. Server-authoritative and replicated where clients need it. Persistable (SaveGame). | Contain game rules; know about Controllers, ViewModels, widgets or actors |
| **Controller** | Many small, domain-specific controllers (e.g. Enemy, Time, Dialogue controller), GAS abilities | The **only** layer that writes to the Model. Runs on the server for replicated state. Initialization and tick order between controllers is declared with the order-of-operations macro (9.1). | Hold game state of its own; know about widgets; be monolithic |
| **ViewModel** | Classes derived from `UMVVMViewModelBase` | Converts Model data into displayable UI state; forwards user actions to Controllers | Write the Model directly; contain game rules; be replicated; reference widgets |
| **View** | UMG widgets; world presentation actors (characters, NPCs, enemies, spawners, interactables) | Displays state; forwards input/events to Controllers (world) or ViewModel (UI) | Contain game rules; hold authoritative game state |

**Controller naming:** controllers are named `U<Domain>Controller` (e.g. `UEnemyController`, `UTimeController`, `UDialogueController`). Unreal's actor controllers keep their engine naming and always carry their kind in the name (`A<Name>PlayerController`, `A<Name>AIController`), so the two are never confused.

**Model location:** the Model lives under the GameState and is organized in domain sub-categories:
```
GameState
  -> Model
       -> Enemy  -> ActorList
       -> Time   -> DayTime
       -> ...
```
The concrete Unreal types (e.g. replicated components / UObjects) are defined in the MVVM/MVC plan.

All game data lives in the Model wherever possible. Which data is shared between both co-op players and which is per player is decided in the MVC / MVVM plan.

### 8.2 Data Flow

UI:
```
Player input (button click)
  -> View calls a ViewModel function (command)
  -> ViewModel calls a Controller (server request if needed)
  -> Controller validates and writes the Model (server)
  -> Model replicates to clients
  -> ViewModel updates its FieldNotify property
  -> View Binding updates the widget
```

World:
```
World event (e.g. player enters a trigger, enemy is hit)
  -> Actor calls a Controller
  -> Controller validates and writes the Model (server)
  -> Model replicates to clients
  -> Actors / ViewModels read the new state and present it
```

Data flows in one direction: **Model -> ViewModel / Actors -> View**. Changes flow only through **Controller -> Model**.

### 8.3 MVVM Rules (UI)

- **ViewModel properties** are declared with `FieldNotify` and are **read-only for the View**:
  ```cpp
  /** Current health shown in the HUD health bar. */
  UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Health")
  float CurrentHealth = 0.0f;
  ```
  Setters are `private`/`protected` C++ functions using `UE_MVVM_SET_PROPERTY_VALUE`. Never expose mutable state to the View.
- **Derived values** are `UFUNCTION(BlueprintPure, FieldNotify)` getters; broadcast with `UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED` when their inputs change.
- **User actions** are `UFUNCTION(BlueprintCallable)` functions on the ViewModel, named as commands (`RequestUseItem`, `SelectDialogueOption`).
- **Small ViewModels:** one ViewModel per UI concern (e.g. `UHealthViewModel`, `UInventoryViewModel`, `UDialogueViewModel`). No "god" ViewModels.
- **Dependency injection:** ViewModels get their Model/Controller references from outside, never by searching the world. Use the plugin's creation types (`Manual`, `CreateInstance`, `GlobalViewModelCollection`, `PropertyPath`, `Resolver`) as decided per feature in the plan. Access goes through interfaces or subsystems, not concrete actor casts.
- **Lifecycle:** ViewModels hold `TWeakObjectPtr` to Model objects and handle Model objects that are destroyed or not yet replicated.
- **Testability:** ViewModels must be testable without widgets.

### 8.4 Pitfalls to Avoid

- Game rules in widgets, Widget Blueprints or presentation actors.
- Anything other than a Controller writing to the Model.
- Game state stored in actors or widgets instead of the Model.
- Widgets reading from components, PlayerState, or GAS directly.
- Public setters / `BlueprintReadWrite` on ViewModel state.
- Massive ViewModels or Controllers covering several concerns.
- Finding dependencies via `GetAllActorsOfClass` or hard casts.
- Assuming Model data is present on clients before replication has arrived.

## 9. Architecture

**Decided:**
- **Combat / abilities:** Gameplay Ability System (GAS).
- **Dialogue:** own dialogue plugin. Dialogues are stored as DataAssets and edited in a **custom visual graph editor** (node-based, Blueprint-like), so complex branching dialogues can be built visually.
- **Modularity:** larger, generic features each get their own plugin or module: **Dialogue, Combat, Quest, UI**. Game-specific content and glue code stays in the `SimpleLifeSim` game module.
- **AI (enemies, bosses, NPCs):** StateTree.
- **Paradigm:** Model / Controller / Presentation with MVVM for the UI (section 8).

- **Dependency direction:** the game module depends on plugins, **never the other way round**. Dependencies between plugins are clarified per feature in its plan.
  - Layering: Engine / Epic plugins -> `KeyCore` (core) -> feature plugins (Dialogue, Combat, Quest, UI) -> game module `SimpleLifeSim`.
  - `KeyCore` depends only on Epic plugins (`ModularGameplay`, `ModelViewViewModel`, `FieldNotification`). It never depends on a feature plugin or on the game module.
  - Feature plugins may depend on `KeyCore` but not on each other unless their plan says so.

### 9.0 Core Framework `KeyCore`

- **Location:** `Plugins/KeyCore/` (core plugin, module `KeyCore`). Holds the generic Model / Controller infrastructure of section 8. Game-specific classes do not go here.
- **Enabled in `SimpleLifeSim.uproject`** (a project plugin is only built and loaded when enabled there).
- **Classes (introduced by the MVVM foundation plan, `Docs/Plans/MVVMIntroduction.md`):**
  - `UModelComponent` (Model base, replicated `UGameStateComponent`), `UModelContainer` (access to Sub-Models via `Get<T>()`).
  - `UControllerBase` (Controller base: `Initialize`, `Deinitialize`, optional `Tick`).
  - `UControllerOrchestrator` (`UTickableWorldSubsystem`, authority only, owns and runs the Controllers).
  - `UControllerSettings` (`UDeveloperSettings`, ordered Controller class list).
- **Log category:** one per system, added with the classes that need it (plan step 3/4).

### 9.1 Planned Foundational Work (in this order)

Work is tracked in GitHub Issues / GitHub Projects (section 13).

1. **Epic: Unreal Setup** - Claude setup, Git/GitHub, local build scripts, server target, MCP connection between Claude and the Unreal Editor.
2. **Epic: MVC / MVVM** - Model / Controller / MVVM introduction - the first code in the project. Requires a plan (`Docs/Plans/MVVMIntroduction.md`) and approval.
3. **Order-of-operations system** - a macro to declare dependencies and execution order.
   - Scope: **initialization order and tick order** of Controllers.
   - Validation: **at compile time**. Cycles and impossible orders abort the build with a **readable compiler message**.
   - Technical constraints (C++20 `static_assert` only accepts literal messages, interaction with Unreal tick groups and subsystem initialization) are worked out in its plan.


### 9.2 TODO: Game Design Document (GDD)

**Status:** planned, not started. **Blocked:** the GitHub repository `BauerrangerCode/SimpleLifeSim` does not exist yet. Once it is created and the Claude GitHub app has access, Claude sets up the GDD there.

**Decided:**
- Format: Markdown files in the repository under `Docs/GDD/` (free, readable/writable by Claude, reachable online and on mobile via GitHub without the user's PC).
- Images in `Docs/GDD/images/`, tracked via **Git LFS** (already covered by the existing `*.png` / `*.jpg` rules in `.gitattributes`).
- Diagrams (flows, state machines, boss phases) preferably as Mermaid in Markdown.
- Content is never invented: known decisions are filled in, everything else gets `//TODO claude:` markers.

**Planned structure:**
```
Docs/GDD/
  README.md              # overview, table of contents, status per chapter
  01_Vision.md           # pitch, target audience, pillars, genre, references
  02_CoreLoop.md         # gameplay loop, progression
  03_Combat/
    README.md            # shared systems (GAS, stats, damage, targeting)
    Melee.md
    Ranged.md
    Magic.md
  04_Survival.md         # hunger, temperature - explicitly NO stamina
  05_Crafting.md
  06_Building.md
  07_Characters.md       # 15 story characters
  08_Enemies.md          # enemy types, AI (StateTree)
  09_Bosses.md           # 5 bosses with multiple phases
  10_Story_Quests.md
  11_World_Levels.md
  12_UI_UX.md
  13_Multiplayer.md      # co-op up to 2 players (online + split-screen)
  14_Audio_Art.md
  images/                # Git LFS
```

**Steps (once the repository exists):**
- [ ] User creates the private repository `BauerrangerCode/SimpleLifeSim` and grants the Claude GitHub app access.
- [ ] Inspect the repository (existing `.gitattributes`, `Docs/`).
- [ ] Create branch `gdd/initial-skeleton`.
- [ ] Create all GDD files with a common template; fill in known decisions, mark open points with `//TODO claude:`.
- [ ] Check that all image formats used in `Docs/GDD/images/` are covered by LFS rules in `.gitattributes` (e.g. `*.gif`, `*.webp` are not yet).
- [ ] Commit (`#GDD, Setup, Claude Code: Initial GDD skeleton with combat sub-chapters, survival, crafting and building`) and push the branch (push requires confirmation). The user decides on merging.

---

## 10. Project Structure

```
SimpleLifeSim/
  CLAUDE.md
  .claude/settings.json      # permissions
  .claude/skills/            # project skills: create-plan, review-against-guidelines, commit-message, todo-report
  .gitignore / .gitattributes
  .ignore                    # excluded from Claude's search (ripgrep)
  Config/                    # .ini files (text, editable)
  Content/                   # binary assets (.uasset/.umap) - not readable as text
  Docs/GDD/                  # Game Design Document (Markdown, images via LFS) - see 9.2
  Docs/Plans/                # one plan file per feature
  Plugins/                   # KeyCore (core framework, see 9.0); generic feature plugins (Dialogue, Combat, Quest, UI)
  Source/SimpleLifeSim/      # game module
```

Source and Content are organized **by feature**, e.g. `Characters/`, `Combat/`, `AI/`, `Dialogue/`, `Quests/`, `UI/`. Within a feature, MVVM layers are separated, e.g. `UI/Health/Model`, `UI/Health/ViewModels`, `UI/Health/Views` (exact layout is defined in the MVVM plan).

---

## 11. Build and Verification

Build the editor target (Windows, run from the project root in `cmd`):
```
..\Engine\Build\BatchFiles\Build.bat SimpleLifeSimEditor Win64 Development -Project="%CD%\SimpleLifeSim.uproject" -WaitMutex
```

- Claude can only run builds when it runs natively on Windows (Claude Code started in this folder on the user's PC). In a Linux shell it cannot compile; then the user builds and pastes the output.
- After every implementation step: build, report warnings and errors honestly, fix them before continuing.
- Never claim "it compiles" without a build output that shows success.

- **CI:** for now, **local build and test scripts only**. A CI system is decided later. (Note: Horde's build automation requires Perforce and does not support Git - see `Engine/Source/Programs/Horde/Docs/Faq.md`.)

### 11.1 Automated Tests

Unreal Automation tests are used for logic that can be tested without manual play. Proposed systems:
- Order-of-operations system (dependency resolution, cycle detection, error output)
- ViewModels (state mapping from Model, commands)
- Dialogue graph evaluation (branching, conditions, consequences)
- Quest state machine (progress, completion, failure, save/load)
- GAS calculations (damage formulas, execution calculations, attribute clamping)
- Boss phase transitions (thresholds, phase order)
- Save/load serialization
- Data validation of DataAssets (`IsDataValid`)

**Frameworks:**
- **Automation Spec** (`BEGIN_DEFINE_SPEC`) for logic, Model, ViewModels and data validation.
- **Functional Tests** (test actors placed in test levels) for behavior in the world, including two-player tests.
- **Compile-time checks** (`static_assert`) for the order-of-operations system.

---

## 12. Files Excluded from Claude

Binary assets, build artifacts, caches and local data are excluded via:
- `.claude/settings.json` -> `permissions.deny` (blocks `Read`/`Edit` on these paths),
- `.ignore` (excludes them from search results).

Excluded: `Binaries/`, `Intermediate/`, `Saved/` (**except `Saved/Logs/`**, which is readable for debugging editor, build and crash logs; Claude never edits it), `DerivedDataCache/`, `.vs/`, `*.uasset`, `*.umap`, `*.ubulk`, `*.uexp`, `*.pak`, `*.pdb`, `*.dll`, `*.exe`, `*.lib`.

Never read or output secrets (API keys, credentials, tokens). If such data is found, stop and tell the user.

---

## 13. Version Control

- **Git with Git LFS, hosted in the private GitHub repository `SimpleLifeSim`.** Only the project folder is versioned, never the engine (Epic's EULA forbids publishing engine source).
- Large data from `Content/` is **not** pushed to GitHub. A separate storage solution for art assets is decided later, when art assets exist.
- `.gitignore` and `.gitattributes` are in the project root.

- **`Content/Art/` is excluded from Git** (meshes, textures, audio, animations go there). Maps, Blueprints and DataAssets elsewhere in `Content/` are versioned via LFS.

**Commit message convention** (English):
```
#<Epic name>, #<Story issue number>, <Coder>: <What was done>
<Why it was changed - required if the change does not come from a design/plan>
```

- `<Coder>` is `Claude Code` for commits written by Claude.

### 13.1 Planning: GitHub Issues + GitHub Projects (single source of truth)

- **GitHub is the single source of truth for Epics and Stories.** No parallel backlog files. `Docs/Backlog.md` is only the seed for the initial import and is replaced by a pointer to GitHub afterwards (Story E01-S06).
- **Epic** = issue with label `epic`, title `E<NN> - <Name>`. Epics are added step by step; not all are known in advance.
- **Story** = issue with label `story`, linked as **sub-issue** of its Epic. Body contains description and acceptance criteria (checklist).
- **Status** = field in the GitHub Project: `Proposed` -> `Approved` -> `In Progress` -> `Done` (or `Rejected`).
- New Stories are created by Claude with status `Proposed` and only become `Approved` through the user.
- Plans stay in the repo (`Docs/Plans/`); the Story links to its plan file.
- Claude accesses GitHub via the GitHub CLI (`gh`). Details in the `github-backlog` skill.

- **Story reference in commits:** the GitHub issue number of the Story (e.g. `#UnrealSetup, #12, Claude Code: ...`), so GitHub links the commit to the issue automatically.

---

## 14. Definition of Done

A task is done only when all of these are true:
- [ ] The approved plan is fully implemented, or deviations were approved.
- [ ] Code follows sections 7 and 8 (Epic standard, Clean Code, no hardcoded values, MVVM).
- [ ] Multiplayer behavior is implemented and tested as described in the plan.
- [ ] The project builds without new warnings or errors (with build output as proof).
- [ ] Acceptance criteria are verified as described in the plan.
- [ ] All new `//TODO claude:` markers are listed for the user.
- [ ] The plan file is updated with the final state.
- [ ] Every completed step is committed; no uncommitted work is left.
