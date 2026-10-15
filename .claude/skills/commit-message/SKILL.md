---
name: commit-message
description: Write a commit message following the project convention. Use whenever a commit is prepared.
---

# Commit Message

Format (English):
```
#<Epic name>, #<Story issue number>, <Coder>: <What was done>
<Why it was changed - required if the change does not come from a design/plan>
```

- `<Story issue number>` is the GitHub issue number of the Story, e.g. `#12` (GitHub links the commit automatically).
- `<Coder>` is `Claude Code` for commits written by Claude.
- Take Epic and Story from GitHub Issues (see the `github-backlog` skill). If the change does not map to a Story, ask the user. Do not invent Epics or Stories.
- First line: what was done, in imperative mood.
- Second line: why it was changed, if the change is not part of an approved design/plan.
- Show the message to the user before committing. Pushing always requires the user's confirmation.
