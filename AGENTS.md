# AGENTS.md — n-body Repository Workflow

## Mission

Maintain a CUDA/OpenGL N-body simulation demo. Optimize for clarity, correctness, and reduced maintenance burden.

## Canonical Repository Surfaces

| Path | Role |
|------|------|
| `docs/` | Repository-local reader and contributor documentation |
| `site/` | GitHub Pages showcase surface |
| `.github/` | CI, Pages, issue/PR templates, and Copilot instructions |

## Required Workflow

1. Read the relevant headers in `include/nbody/` before changing code.
2. Implement in small coherent batches.
3. Run the highest-value existing checks for the surfaces you changed.

## Project-Specific Rules

- Prefer deletion, consolidation, or redirection over keeping duplicate docs and site mirrors.
- Do not add generic engineering or AI boilerplate; every file must be specific to this project.
- Keep user-facing docs focused on the product and contributor workflow.
- Update required bilingual counterparts when touching primary onboarding docs.

## Tooling and Automation Rules

- Prefer the canonical CMake build path and scripts in `scripts/`.
- Keep dependency versions and GitHub Actions versions pinned or explicitly bounded.
- Default LSP baseline: `clangd` using `compile_commands.json` from the canonical build.

## Documentation Rules

- `README.md` / `README.zh-CN.md` explain the project, quick start, and canonical links.
- `CONTRIBUTING.md` defines the contributor workflow and quality gates.
- `CLAUDE.md`, `AGENTS.md`, and `.github/copilot-instructions.md` must agree on the same operating model.
- GitHub Pages should complement the repository and market the project; it should not mirror every markdown file.

## Working Style

- Be surgical but complete.
- Reuse existing patterns where they are sound; rewrite when the current structure is the problem.
- Fix closely related drift or bugs you uncover while touching a surface.
- Leave the repository easier to understand than you found it.
