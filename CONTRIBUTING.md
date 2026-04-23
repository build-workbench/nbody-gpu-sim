# Contributing to n-body

Thanks for improving the project.

## Core Principles

1. **OpenSpec first** — `openspec/specs/` is the only active source of truth.
2. **No duplicate guidance** — prefer one canonical doc per purpose.
3. **Project-specific quality** — avoid generic templates, vague docs, or ceremonial automation.
4. **Small coherent changes** — keep changes easy to review and merge.

## Canonical Workflow

1. Read the relevant spec files in `openspec/specs/`.
2. If the work changes behavior, workflow, documentation policy, automation, or structure, create or update an OpenSpec change in `openspec/changes/`.
3. Update proposal/design/spec deltas/tasks before implementation.
4. Implement from `tasks.md`, marking each checkbox as it is completed.
5. Run the existing highest-value checks for the surfaces you changed.
6. Use `/review` before finalizing major structural or governance refactors.

## Branching and Merge Discipline

- Prefer short-lived branches or short-lived cloud sessions.
- Avoid long-running local/cloud divergence.
- Merge or rebase frequently enough that the OpenSpec change and implementation stay in sync.
- For broad cleanup work, prefer a longer `autopilot` implementation pass over repeated `/fleet` usage unless the task is genuinely parallel-heavy.

## Documentation Rules

- `README.md` / `README.zh-CN.md` explain the project and quick start.
- `AGENTS.md`, `CLAUDE.md`, and `.github/copilot-instructions.md` must describe the same operating model.
- `docs/` holds canonical repository-local documentation.
- `site/` is the GitHub Pages showcase surface and should complement the repository rather than mirror it.
- Update required bilingual counterparts when changing core specs or primary onboarding docs.

## Engineering Rules

- Prefer the canonical CMake build path and the scripts in `scripts/`.
- Keep dependencies and GitHub Actions versions pinned or explicitly bounded.
- Keep CI and Pages triggers narrow and meaningful.
- Default LSP baseline: `clangd` using the compile database generated from the primary build.
- Keep hooks, plugins, and MCP usage lean and project-specific.

## Local Commands

```bash
./scripts/build.sh
./scripts/test.sh
./scripts/format.sh
```

## Pull Requests

- Explain what changed and why.
- Reference the OpenSpec change when applicable.
- Call out docs, workflow, and repository-structure changes explicitly.
- Note any checks you ran and any environment limitations that prevented a check.
