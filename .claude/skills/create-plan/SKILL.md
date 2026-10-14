---
name: create-plan
description: Create a feature plan in Docs/Plans before any implementation. Use whenever the user requests a new feature, refactoring or non-trivial change.
---

# Create a Feature Plan

No code is written before a plan is approved (CLAUDE.md, section 4).

## Steps

1. **Clarify first.** Ask the user (in German) until goal, scope, acceptance criteria and edge cases are unambiguous. Do not assume anything.
2. **Research the codebase.**
   - Find existing code that can be reused (including the template `Variant_*` code).
   - Check for existing Unreal Engine systems that solve the problem (Unreal technology first). Verify APIs in the engine source (`../Engine/Source`, `../Engine/Plugins`).
3. **Write the plan** to `Docs/Plans/<FeatureName>.md` (English) using the template in CLAUDE.md, section 4. Every plan must state:
   - the layer of every new or changed class: Model, Logic, ViewModel or View (CLAUDE.md, section 8),
   - which values are editor-exposed and which DataAssets are used (no hardcoded values),
   - networking: what is replicated, what runs on server / owning client,
   - any delegate that cannot be avoided, with justification,
   - plugin/module placement and dependency direction,
   - tests (Automation Spec, Functional Test, compile-time checks) and the two-player verification.
4. **List open questions** in the plan. A plan with open questions cannot be approved.
5. **Present the plan** in chat (German summary) and ask for explicit approval.
6. Mark every unclear point with `//TODO claude: <description>` and tell the user.
