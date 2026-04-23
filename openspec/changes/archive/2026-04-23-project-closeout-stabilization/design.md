## Context

The repository already contains the major simulation capabilities, but its surrounding governance layer has drifted. `openspec/` exists alongside legacy `specs/` references, documentation is duplicated across repository and Pages surfaces, workflow automation is noisy without strongly validating the real developer path, and AI/tooling instruction files are inconsistent or missing. The project is not trying to expand scope; it is trying to finish strong and enter a low-maintenance state with a clear, enforceable operating model.

This change is cross-cutting: it touches specs, docs, site content, workflows, editor/tooling config, GitHub metadata, and parts of the engineering baseline. The current working tree also contains in-progress user changes, so the implementation must refine the existing direction rather than reset the repository to an older state.

## Goals / Non-Goals

**Goals:**
- Re-establish `openspec/` as the only active specification system.
- Define a repository governance capability that covers workflow, documentation, automation, and public project surfaces.
- Remove stale or duplicative content so the repository is easier to understand and cheaper to maintain.
- Align CI, Pages, hooks, LSP, and assistant instructions with one canonical development path.
- Prepare a final closeout workflow optimized for long-running `autopilot` execution and periodic `/review`, without depending on `/fleet`.

**Non-Goals:**
- Adding new simulation algorithms or broadening product scope.
- Rebuilding the rendering or physics architecture unless a bug fix directly requires it.
- Introducing a heavy MCP/plugin stack that increases context cost without clear project-specific value.
- Advertising archival/low-maintenance intent to external readers in user-facing docs.

## Decisions

### Decision: Treat the current unstaged worktree as the new baseline

The repository already contains meaningful in-progress restructuring. The implementation will reconcile and refine those changes instead of avoiding them or trying to restore a previously cleaner-looking state.

**Alternatives considered:**
- Freeze all modified files and work around them: rejected because most target surfaces are already modified and would create duplicated effort.
- Revert and restart from HEAD: rejected because it would discard user intent and violate the non-destructive working rule.

### Decision: Consolidate governance into OpenSpec + a small set of canonical instruction files

OpenSpec will remain the governing change system. Repository-level workflow instructions will be consolidated into a small canonical set: `AGENTS.md`, a new `CLAUDE.md`, and generated Copilot instructions, all aligned to the same process.

**Alternatives considered:**
- Keep separate assistant-specific documents with overlapping project summaries: rejected because that caused drift already.
- Use only one assistant-specific file and ignore the others: rejected because the repository is intentionally used with multiple tools.

### Decision: Prefer deletion and consolidation over preservation of duplicate docs

If two docs serve the same purpose, one will become canonical and the other will be removed, redirected, or reduced to a short index. Pages content should complement the repository, not mirror every markdown file.

**Alternatives considered:**
- Preserve all documents and “just update them”: rejected because it increases maintenance cost and invites future drift.
- Move everything into Pages: rejected because repository-local contributor and specification docs still need to exist close to the code.

### Decision: Make workflows validate the real path, not ceremonial checks

CI and Pages automation will be narrowed to meaningful triggers and useful checks. Preference goes to validating the documented build/test/docs path over string-based heuristics or cosmetic workflows.

**Alternatives considered:**
- Keep broad triggers for maximum visibility: rejected because they create noisy runs with little signal.
- Add many specialized workflows: rejected because the project is in closeout mode and should reduce maintenance burden.

### Decision: Standardize dev tooling around `clangd` and generated compile commands

The project will optimize for a single robust LSP baseline: `clangd` driven by `compile_commands.json` from the canonical CMake build. Secondary language servers for Markdown/YAML/CMake/Bash are optional and lightweight.

**Alternatives considered:**
- Tool-specific language tooling per assistant/editor: rejected because it fragments setup and knowledge.
- Heavy editor-specific configuration: rejected because the project should stay portable across Copilot, Claude Code, Codex, and standard editors.

### Decision: Keep MCP and plugin usage minimal

The repository will prefer native GitHub, OpenSpec, and local tool integrations before adding MCP or plugin layers. MCP will only be added if it clearly replaces repetitive work without large context overhead.

**Alternatives considered:**
- Add a broad MCP stack for maximum capability: rejected because the project priority is lean closeout, not experimentation.

## Risks / Trade-offs

- **[Risk] Aggressive cleanup may remove content that still has niche value** → Mitigation: audit first, keep a canonical replacement path before deleting user-visible material.
- **[Risk] Structural rewrites can collide with the existing dirty worktree** → Mitigation: treat current edits as baseline and reconcile file-by-file rather than overwrite wholesale.
- **[Risk] Tightening workflows may expose latent code or docs bugs** → Mitigation: plan includes a dedicated bug sweep after governance/doc/tooling changes land.
- **[Risk] New governance docs can become over-engineered** → Mitigation: bias toward short, project-specific guidance and reject generic templates that do not change contributor behavior.

## Migration Plan

1. Create and approve the umbrella change artifacts.
2. Normalize OpenSpec references and define the new repository governance requirements.
3. Consolidate or remove stale docs and assistant instructions.
4. Rationalize engineering configuration, workflows, hooks, and LSP/editor defaults.
5. Redesign Pages and update GitHub metadata.
6. Run a focused validation/bug sweep and fix issues revealed by the cleanup.
7. Archive the change after the final closeout pass is complete.

Rollback strategy is file-level and incremental: because this is a repository-governance refactor, changes should be landed in coherent batches so any problematic surface can be reverted independently without rolling back the entire cleanup.

## Open Questions

- No blocking product questions remain for the cleanup itself.
- During implementation, any uncertainty should prefer the lower-maintenance option unless it would materially reduce project clarity or correctness.
