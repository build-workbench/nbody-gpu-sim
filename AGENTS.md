# AGENTS.md

CUDA/OpenGL N-body simulation demo. Optimize for clarity, correctness, low maintenance.

## Workflow

1. Read relevant headers in `include/nbody/` before changing code.
2. Implement in small coherent batches.
3. Run the highest-value existing checks for the surfaces you changed.

## Rules

- Prefer deletion and consolidation over keeping duplicates.
- No generic boilerplate; every file must be project-specific.
- Use canonical CMake build + `scripts/`. LSP: `clangd` with `compile_commands.json`.
- Keep dependency and Actions versions pinned.
