# AGENTS.md — n-body Repository Workflow

## Mission

Maintain and stabilize a CUDA/OpenGL N-body simulation project that is now in a final quality-focused cleanup phase. Optimize for clarity, correctness, and reduced maintenance burden rather than feature expansion.

## Canonical Repository Surfaces

| Path | Role |
|------|------|
| `openspec/specs/` | Active requirements and repository rules |
| `openspec/changes/` | Active proposals, designs, delta specs, and task lists |
| `openspec/changes/archive/` | Completed changes |
| `specs-archived/` | Historical-only material; never use as the active source of truth |
| `docs/` | Repository-local reader and contributor documentation |
| `site/` | GitHub Pages showcase surface |
| `.github/` | CI, Pages, issue/PR templates, and Copilot instructions |

## Capability Map

- `simulation-core.md` — particle data, force calculation, Barnes-Hut
- `force-computation.md` — spatial hash and integration
- `visualization.md` — CUDA/OpenGL interop and rendering
- `simulation-control.md` — runtime control, serialization, validation
- `quality-attributes.md` — performance, reliability, maintainability, validation quality
- `repository-governance.md` — OpenSpec workflow, docs policy, automation quality, GitHub presentation, closeout rules

## Required Workflow

1. Read the relevant files in `openspec/specs/` before changing code, docs, workflows, or project behavior.
2. If the work changes behavior, workflow, docs policy, automation, or repository structure, create or update an OpenSpec change in `openspec/changes/` before implementation.
3. Keep proposal, design, specs, and tasks aligned. Do not implement from vague intent alone.
4. Implement in small coherent batches and mark task checkboxes immediately.
5. Run the highest-value existing checks for the surfaces you changed.
6. Use `/review` before finalizing major structural, workflow, or documentation refactors.

## Project-Specific Rules

- Treat `openspec/` as the only active specification system.
- Historical specs in `specs-archived/` are for reference only.
- During the current cleanup effort, treat existing unstaged repository edits as the working baseline unless the user explicitly says otherwise.
- Prefer deletion, consolidation, or redirection over keeping duplicate docs and site mirrors.
- Do not add generic engineering or AI boilerplate; every file must be specific to this project.
- Keep user-facing docs focused on the product and contributor workflow; do not announce archive/maintenance intent publicly.
- Update required bilingual counterparts when touching core specs or primary onboarding docs.

## Tooling and Automation Rules

- Prefer the canonical CMake build path and scripts in `scripts/`.
- Keep dependency versions and GitHub Actions versions pinned or explicitly bounded.
- Prefer high-signal workflows over broad or ceremonial checks.
- Default LSP baseline: `clangd` using `compile_commands.json` from the canonical build.
- Keep MCP/plugin usage minimal; prefer native OpenSpec, GitHub, and editor capabilities first.
- Prefer longer `autopilot` execution over routine `/fleet` usage unless the work is genuinely parallel-heavy.

## Documentation Rules

- `README.md` / `README.zh-CN.md` explain the project, quick start, and canonical links.
- `CONTRIBUTING.md` defines the contributor workflow and quality gates.
- `CLAUDE.md`, `AGENTS.md`, and `.github/copilot-instructions.md` must agree on the same operating model.
- `QWEN.md` is a compatibility stub only if retained; it must not become an independent source of instructions.
- GitHub Pages should complement the repository and market the project; it should not mirror every markdown file.

## Working Style

- Be surgical but complete.
- Reuse existing patterns where they are sound; rewrite when the current structure is the problem.
- Fix closely related drift or bugs you uncover while touching a surface.
- Leave the repository easier to understand than you found it.
