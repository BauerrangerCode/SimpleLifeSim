---
name: todo-report
description: List all open "//TODO claude:" markers in the project. Use at the end of every task and whenever the user asks for open points.
---

# TODO Report

1. Search the project for the exact marker `//TODO claude:` in `Source/`, `Plugins/`, `Config/`, `Docs/`, `CLAUDE.md` and project config files. Exclude `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`.
2. Report in German, grouped by file: file path, line number, the marker text.
3. Highlight markers that were added in the current session.
4. Do not resolve markers on your own; each one needs the user's input.
