# CLAUDE.md — n-body Project Notes

Follow `AGENTS.md` as the primary repository workflow document.

## Claude-Specific Priorities

- Read `openspec/specs/*.md` before editing code, docs, workflows, or automation.
- Use OpenSpec changes for any behavior, workflow, documentation, or repository-structure change.
- Treat the current cleanup as a repository-governance refactor: prefer consolidation and deletion over preserving duplicate guidance.
- Keep outputs concise and project-specific. Avoid generic engineering advice.
- Prefer one long-running implementation pass over repeated fragmented sessions.
- Use `/review` before finalizing major structural refactors.

## Editing Rules

- Keep `AGENTS.md`, `.github/copilot-instructions.md`, and this file aligned.
- Keep README, core specs, and key onboarding surfaces in bilingual parity when required.
- Prefer `clangd` + CMake-generated `compile_commands.json` as the editor/LSP baseline.
- Keep MCP/plugin additions minimal unless they replace clear repetitive work for this project.
- Do not describe the project as archived or low-maintenance in public-facing docs.
