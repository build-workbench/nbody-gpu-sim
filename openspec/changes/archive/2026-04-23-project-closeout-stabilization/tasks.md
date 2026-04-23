## 1. Governance Baseline

- [x] 1.1 Audit legacy spec, doc, site, and workflow surfaces and classify each file as keep, rewrite, merge, redirect, or remove
- [x] 1.2 Normalize OpenSpec references so `openspec/` is the only active spec system across repository guidance and automation
- [x] 1.3 Restore required bilingual parity for active specs, including the missing `force-computation.zh-CN.md`

## 2. Canonical Instructions and Core Docs

- [x] 2.1 Rewrite `README.md` and `README.zh-CN.md` so they point to the canonical repo structure, current specs, and real project value
- [x] 2.2 Rewrite `CONTRIBUTING.md`, add `CLAUDE.md`, and generate Copilot instructions so all assistant and contributor workflows align
- [x] 2.3 Consolidate or retire stale assistant/context docs such as `QWEN.md` and remove broken or duplicate references across docs and examples

## 3. Pages and GitHub Presentation

- [x] 3.1 Redesign the GitHub Pages home, docs, and changelog experience into a project showcase rather than a repository mirror
- [x] 3.2 Simplify site content so published pages complement the repository instead of duplicating every markdown surface
- [x] 3.3 Update GitHub About metadata with a sharper description, curated topics, and the Pages URL using `gh`

## 4. Engineering and Automation

- [x] 4.1 Rationalize `CMakeLists.txt`, scripts, dependency pinning, version anchors, and example wiring around one canonical build path
- [x] 4.2 Simplify CI and Pages workflows so triggers are scoped and checks validate supported repository behavior
- [x] 4.3 Add lightweight hooks, LSP/editor defaults, and project-level dev tooling guidance centered on `clangd` and generated compile commands

## 5. Validation and Closeout

- [x] 5.1 Run the highest-value existing checks supported by the environment and record any failures exposed by the cleanup
- [x] 5.2 Fix confirmed code, docs, workflow, and site bugs uncovered by validation and link/path auditing
- [x] 5.3 Document the final closeout operating model, complete the change checklist, and prepare the change for archive
