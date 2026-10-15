---
name: github-backlog
description: Read and manage Epics and Stories in GitHub Issues / GitHub Projects, the single source of truth for planning. Use when the user asks about the backlog, when a Story is started or finished, or when new Stories are proposed.
---

# GitHub Backlog

GitHub Issues + GitHub Projects are the single source of truth (CLAUDE.md, section 13.1). Never keep a parallel backlog in files.

## Conventions

- Epic: issue, label `epic`, title `E<NN> - <Name>`.
- Story: issue, label `story`, sub-issue of its Epic. Body:
  ```markdown
  ## Description
  ...
  ## Acceptance Criteria
  - [ ] ...
  ## Plan
  Docs/Plans/<FeatureName>.md (once it exists)
  ```
- Status (Project field): `Proposed`, `Approved`, `In Progress`, `Done`, `Rejected`.

## Rules

- Claude creates new Stories only with status `Proposed`. Only the user sets `Approved` (or tells Claude to).
- Never close, delete or re-prioritize issues without the user's instruction.
- Before starting work, check that the Story is `Approved`; set it to `In Progress`. After the Definition of Done is met, report to the user; set `Done` only after the user agrees.
- Report backlog contents to the user in German; issue texts are written in English.

## Tooling

- GitHub CLI `gh` must be installed and authenticated (`gh auth login`). Project commands need the `project` scope (`gh auth refresh -s project`).
- Typical commands: `gh issue list`, `gh issue view <n>`, `gh issue create`, `gh issue edit`, `gh project item-list`, `gh project item-add`, `gh project item-edit`.
- Sub-issues: check whether the installed `gh` version supports them directly; otherwise use `gh api graphql` with the `addSubIssue` mutation. Verify before use; do not guess.
- If a command fails or a feature is not available, stop and tell the user. Do not work around it with a parallel file backlog.
