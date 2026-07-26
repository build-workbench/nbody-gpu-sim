# Contributing to n-body

Thanks for improving the project.

## Core Principles

1. **No duplicate guidance** — prefer one canonical doc per purpose.
2. **Project-specific quality** — avoid generic templates, vague docs, or ceremonial automation.
3. **Small coherent changes** — keep changes easy to review and merge.

## Canonical Workflow

1. Read the relevant headers in `include/nbody/` before changing code.
2. Implement in small coherent batches.
3. Run the existing highest-value checks for the surfaces you changed.

## Branching and Merge Discipline

- Prefer short-lived branches.
- Avoid long-running divergence.

## Documentation Rules

- `README.md` / `README.zh-CN.md` explain the project and quick start.
- `AGENTS.md` is the single AI assistant guidance file.
- `site/` is the GitHub Pages showcase surface.
- Update required bilingual counterparts when changing primary onboarding docs.

## Engineering Rules

- Prefer the canonical CMake build path and the scripts in `scripts/`.
- Keep dependencies and GitHub Actions versions pinned or explicitly bounded.
- Keep CI and Pages triggers narrow and meaningful.
- Default LSP baseline: `clangd` using the compile database generated from the primary build.

## Local Commands

```bash
./scripts/build.sh
./scripts/test.sh
./scripts/format.sh
```

## Pull Requests

- Explain what changed and why.
- Call out docs, workflow, and repository-structure changes explicitly.
- Note any checks you ran and any environment limitations that prevented a check.
