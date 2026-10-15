# Backlog - SimpleLifeSim (import seed)

> **This file is temporary.** It is the seed for the initial import into GitHub Issues / GitHub Projects (Story E01-S06). After the import, GitHub is the single source of truth and this file is replaced by a pointer to the GitHub Project.

Epics are added step by step. Stories proposed by Claude have the status `Proposed` and only become `Approved` after the user's approval.

**Status values:** `Proposed` -> `Approved` -> `In Progress` -> `Done` (or `Rejected`)

**Commit reference:** `#<Epic name>, #<Story issue number>, <Coder>: <What was done>`
Example: `#UnrealSetup, #3, Claude Code: Initialize Git repository with LFS`
(The `E01-S02` IDs in this file are only used until the import into GitHub.)

---

## E01 - Unreal Setup

Goal: Claude and the development environment are fully set up, so feature work can start.

### E01-S01 - Run Claude Code natively on Windows
- **Status:** Proposed
- **Description:** Install Claude Code on Windows and start it in the project folder, so Claude can build, test and use Git directly.
- **Acceptance criteria:**
  - [ ] Claude Code runs in `F:\UnrealEngine\UnrealEngine-release\SimpleLifeSim`.
  - [ ] `CLAUDE.md` is loaded at session start.
  - [ ] Project skills (`create-plan`, `review-against-guidelines`, `commit-message`, `todo-report`) are available.
  - [ ] `/permissions` shows the allow / ask / deny rules from `.claude/settings.json`, and a read of a denied path (e.g. a `.uasset`) is blocked.

### E01-S02 - Git repository with LFS on GitHub
- **Status:** Proposed
- **Description:** Put the project folder (not the engine) under Git with Git LFS and push it to a GitHub repository.
- **Acceptance criteria:**
  - [ ] Git LFS is installed and `.gitattributes` is active.
  - [ ] `Content/Art/`, build artifacts and caches are not tracked.
  - [ ] The first commit follows the commit convention.
  - [ ] Push to GitHub done (after the user's confirmation).

- **Visibility:** private.

- **Repository name:** `SimpleLifeSim`.

### E01-S03 - Local build and test scripts
- **Status:** Proposed
- **Description:** Scripts to build the editor target, build the server target, and run automation tests from the command line with readable output.
- **Acceptance criteria:**
  - [ ] One command each for: build editor, build server, run tests.
  - [ ] Scripts return a non-zero exit code on failure.
  - [ ] Claude can run them and report the result.

### E01-S04 - Dedicated server build target
- **Status:** Proposed
- **Description:** Add a `Server` build target, required for online play and for the local background server used by split-screen.
- **Acceptance criteria:**
  - [ ] `SimpleLifeSimServer.Target.cs` exists and the server target builds.
  - [ ] A dedicated server starts locally and a client can connect.

### E01-S05 - Connect Claude to the Unreal Editor via MCP
- **Status:** Proposed
- **Description:** Use Epic's own **Unreal MCP** plugin (`ModelContextProtocol`, Experimental in UE 5.8.3, already enabled in the `.uproject` together with `AllToolsets`) so Claude Code can talk to the running editor. According to the plugin source, it runs an HTTP MCP server (default: port `8000`, path `/mcp`) with an auto-start setting.
- **Acceptance criteria:**
  - [ ] The MCP server starts with the editor (auto-start setting configured in project settings).
  - [ ] Claude Code is connected to the server via a project-level MCP configuration.
  - [ ] Claude can list the available tools and a read-only test call succeeds.
  - [ ] Which toolsets are enabled is documented (only the ones needed).

### E01-S06 - GitHub Issues + Project as single source of truth for planning
- **Status:** Proposed
- **Description:** Set up GitHub Issues and a GitHub Project as the single source of truth for Epics and Stories, and import this file.
- **Acceptance criteria:**
  - [ ] GitHub CLI `gh` is installed and authenticated on Windows with the `project` scope.
  - [ ] Labels `epic` and `story` exist.
  - [ ] A GitHub Project exists with a `Status` field (`Proposed`, `Approved`, `In Progress`, `Done`, `Rejected`).
  - [ ] All Epics and Stories from this file exist as issues, Stories linked as sub-issues of their Epic, with the correct status.
  - [ ] This file is replaced by a pointer to the GitHub Project.
  - [ ] Claude can read and update the backlog using the `github-backlog` skill.

---

## E02 - MVC / MVVM

Goal: Introduce the Model / Controller / Presentation architecture with MVVM for the UI (CLAUDE.md, section 8) as the first code in the project.

### E02-S01 - Plan for the MVC / MVVM introduction
- **Status:** Proposed
- **Description:** Write `Docs/Plans/MVVMIntroduction.md` using the `create-plan` skill.
- **Acceptance criteria:**
  - [ ] The plan defines the concrete Unreal types for Model, Controller, ViewModel and View.
  - [ ] The plan contains no open questions.
  - [ ] The user has approved the plan.

Further Stories of this Epic are derived from the approved plan.
