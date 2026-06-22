# CLAUDE.md — n-body Project Notes

Follow `AGENTS.md` as the primary repository workflow document.

## Claude-Specific Priorities

- Read the relevant headers in `include/nbody/` before editing code.
- Keep outputs concise and project-specific. Avoid generic engineering advice.
- Prefer one long-running implementation pass over repeated fragmented sessions.

## Editing Rules

- Keep `AGENTS.md`, `.github/copilot-instructions.md`, and this file aligned.
- Keep README and key onboarding surfaces in bilingual parity when required.
- Prefer `clangd` + CMake-generated `compile_commands.json` as the editor/LSP baseline.
