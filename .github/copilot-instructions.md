# GitHub Copilot Instructions for n-body

## Repository Rules

- This is a CUDA/OpenGL N-body simulation demo. Optimize for clarity and correctness.
- Prefer deleting or consolidating duplicate docs instead of keeping mirrored content.

## Development Guidance

- Keep `AGENTS.md`, `CLAUDE.md`, and this file aligned.
- Use the canonical CMake build path and scripts in `scripts/`.
- Prefer `clangd` backed by `compile_commands.json` for C++/CUDA navigation.
- Keep workflow changes high-signal and tightly scoped.

## Documentation Guidance

- Update bilingual counterparts when touching primary onboarding docs.
- Keep `docs/` canonical for repository-local docs.
- Keep `site/` focused on project presentation rather than mirroring every repository markdown file.
