---
name: review-against-guidelines
description: Review changed or specified C++ code against CLAUDE.md (Epic coding standard, Clean Code, no hardcoded values, Model/Logic/MVVM, networking). Use after an implementation step or when the user asks for a review.
---

# Review Against Project Guidelines

Read-only. Do not change code during the review; report findings and let the user decide.

## Checklist

**Epic coding standard (wins on conflict)**
- Type prefixes, PascalCase, `b` prefix for bools, Allman braces, tabs.
- `TObjectPtr` + `UPROPERTY` for UObject members, `const` correctness, `override`, `nullptr`, `TEXT()`.
- Doc comments on every UCLASS / UPROPERTY / UFUNCTION.
- Copyright header (CLAUDE.md 7.1), Epic notice kept on template files.

**Clean Code**
- Small functions doing one thing, no flag arguments, few parameters.
- Descriptive names, no commented-out code, no noise comments.
- No duplication, single responsibility.

**No hardcoded values**
- Every number, string, tag or asset path is editor-exposed, in a DataAsset, in `UDeveloperSettings`, or a Gameplay Tag.
- `UPROPERTY` specifiers: `EditDefaultsOnly`/`BlueprintReadOnly` default, `Category`, clamp/units meta.
- Player-facing text is `FText`.

**Architecture (CLAUDE.md section 8)**
- Only Logic writes the Model. No game rules in widgets or presentation actors.
- ViewModels: read-only FieldNotify state, commands as functions, no widget references, not replicated.
- No `GetAllActorsOfClass` / hard casts to find dependencies.
- Delegates avoided; any remaining one is justified in the plan.

**Networking**
- Server-authoritative writes, correct replication registration, validated server RPCs, explicit authority checks.

**Unreal practices**
- Tick disabled unless justified, log categories, correct use of `check` / `ensure`.

## Output

Report in German: a list of findings with file, line, rule and a concrete fix suggestion, ordered by severity. State clearly what was not checked.
