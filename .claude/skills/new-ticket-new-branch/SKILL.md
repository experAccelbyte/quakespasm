---
name: new-ticket-new-branch
description: Create new git branches for a Jira ticket across selected repos and submodules
---

You are setting up a new git branch for a new ticket or piece of work. Follow these steps **strictly in order**. Do not skip or reorder steps.

## Repo Paths (verified)

| Alias | Path |
|---|---|
| **oss-sample** (outermost) | `d:/ue-oss-sample` |
| **OSS** submodule | `d:/ue-oss-sample/Plugins/AccelByte` |
| **SDK** submodule | `d:/ue-oss-sample/Plugins/AccelByte/AccelByteUe4Sdk` |
| **Test Utilities** submodule | `d:/ue-oss-sample/Plugins/AccelByteTestUtilities` |
| **Networking** submodule | `d:/ue-oss-sample/Plugins/AccelByte/AccelByteNetworkUtilities` |

---

## Step 1 — Confirm Intent

Ask the developer:
> "Do you want me to prepare branch(es) for this ticket?"

- If **No** → say goodbye and exit. Do nothing else.
- If **Yes** → proceed to Step 2.

---

## Step 2 — Which Repos to Branch?

Use the `AskUserQuestion` tool with `multiSelect: true` to present a checkbox-style selection. The developer may pick one or more options.

Question: "Which repos should I branch for this ticket?"

Options:
- `OSS` — OSS submodule (`Plugins/AccelByte`)
- `SDK` — SDK submodule (`Plugins/AccelByte/AccelByteUe4Sdk`)
- `Test Utilities` — TestUtilities submodule (`Plugins/AccelByteTestUtilities`)
- `Networking` — Networking submodule (`Plugins/AccelByte/AccelByteNetworkUtilities`)

Remember the selected options. The selected submodules plus oss-sample will be called **target repos** for the rest of this skill.

Note: oss-sample is **always** included regardless of selection.

---

## Step 3 — Check for Uncommitted Changes

For each target repo, run:

```bash
git -C <repo-path> status --porcelain
```

If **all** target repos are clean → skip to Step 5 (no stashing needed).

If **any** target repo has uncommitted changes → show the developer which repos are dirty, then ask:

> 1. "I'll do nothing — you stash/resolve the changes yourself, then run this skill again."
> 2. "The skill will stash the changes automatically."

- If **1** is chosen → say goodbye and exit. Do nothing else.
- If **2** is chosen → proceed to Step 4.

---

## Step 4 — Form the Branch Name

Before stashing anything, ask the developer:
> "What is the branch name? (format: `AAX-1234-short-description` or `JSC-1234-short-description`)"

Validate it follows the pattern: `[A-Z]+-[0-9]+-[a-z0-9-]+`

If the developer already provided the ticket info as arguments (`$ARGUMENTS`), pre-fill a suggestion and ask them to confirm or edit.

Store the final branch name as **BRANCH_NAME**.

---

## Step 5 — Execute

Perform the following sequence **in order**:

### 5a. Stash changes (only if Step 4 was reached)

For each target repo that had uncommitted changes:
```bash
git -C <repo-path> stash push -m "new-ticket-new-branch: auto-stash before branch <BRANCH_NAME>"
```

### 5b. Checkout master on each target repo

For each target repo (including oss-sample):
```bash
git -C <repo-path> checkout master
git -C <repo-path> pull origin master
```

### 5c. Create new branch on oss-sample

```bash
git -C d:/ue-oss-sample checkout -b <BRANCH_NAME>
```

### 5d. Create new branch on each selected submodule

For each target submodule (not oss-sample):
```bash
git -C <repo-path> checkout -b <BRANCH_NAME>
```

---

## Step 6 — Summary

Report the final state clearly:

- Which repos had changes stashed (if any)
- Which repos are now on branch `<BRANCH_NAME>`
- Any errors or warnings encountered

If everything succeeded, congratulate the developer and remind them:
> "Remember: commit and push each submodule branch independently before pushing oss-sample."
