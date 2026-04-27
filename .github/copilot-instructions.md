# GitHub Copilot Instructions for n-body

## Repository Rules

- Treat `openspec/specs/` as the only active specification source.
- Use or update an OpenSpec change before implementing behavior, workflow, documentation-policy, automation, or structure changes.
- Ignore `specs-archived/` for active development.
- Prefer deleting or consolidating duplicate docs instead of keeping mirrored content.

## Current Project Priorities

- Stabilize and normalize the repository for a final high-quality maintenance baseline.
- Keep guidance project-specific and concise.
- Improve public presentation through README, Pages, and GitHub metadata without mentioning archival intent.

## Development Guidance

- Keep `AGENTS.md`, `CLAUDE.md`, and this file aligned.
- Use the canonical CMake build path and scripts in `scripts/`.
- Prefer `clangd` backed by `compile_commands.json` for C++/CUDA navigation.
- Keep workflow changes high-signal and tightly scoped.
- Prefer long-running `autopilot` work over routine `/fleet` usage unless the task is truly parallel-heavy.

## Documentation Guidance

- Update bilingual counterparts when touching core specs or primary onboarding docs.
- Keep `docs/` canonical for repository-local docs.
- Keep `site/` focused on project presentation rather than mirroring every repository markdown file.
