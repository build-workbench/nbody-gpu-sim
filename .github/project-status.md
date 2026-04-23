# Project Status: Closeout Phase

## Current State

This project is in **closeout and maintenance** phase. All core features are complete and stable.

### Last Major Release
- **Version**: v2.0.0 (2026-03-13)
- **Status**: Stable
- **Next Phase**: Archive-ready

---

## Repository Status Checklist

### ✅ OpenSpec Governance
- [x] Unified `openspec/` as single source of truth
- [x] All bilingual specifications in parity
- [x] Clear capability registration in `openspec.yaml`
- [x] All legacy specs marked as historical-only

### ✅ Documentation
- [x] README.md focused on value and canonical links
- [x] AGENTS.md streamlined to 60 lines
- [x] CLAUDE.md created with Claude-specific guidance
- [x] .github/copilot-instructions.md for Copilot
- [x] CONTRIBUTING.md aligned to OpenSpec workflow
- [x] Stale docs (QWEN.md, etc.) updated to stubs
- [x] Changelog consolidated to canonical directory

### ✅ Build System
- [x] CMakeLists.txt supports CPU-only (`-DNBODY_ENABLE_CUDA=OFF`)
- [x] build.sh auto-detects CUDA availability
- [x] compile_commands.json generated for LSP
- [x] RapidCheck pinned to specific commit
- [x] Example programs wired into build

### ✅ Tooling
- [x] .vscode/settings.json configured for clangd
- [x] .githooks/pre-commit validates code formatting
- [x] .editorconfig uses correct 2-space indent
- [x] setup-hooks.sh initializes git hooks

### ✅ CI/Automation
- [x] pages.yml triggers only on relevant paths
- [x] ci.yml focuses on supported checks
- [x] GitHub About metadata updated with description, topics, homepage
- [x] Workflows simplified and noise-reduced

### ✅ GitHub Presentation
- [x] Description: "High-performance N-body particle simulation..."
- [x] Homepage: https://lessup.github.io/n-body/
- [x] Topics: n-body-simulation, cuda, barnes-hut, gpu-acceleration, particle-physics

---

## Future Development (If Needed)

If further work is required, use the OpenSpec workflow:

```bash
# 1. Create proposal
/opsx:propose "Brief description"

# 2. Edit proposal, design, and tasks
# 3. Implement using long autopilot session
/opsx:apply

# 4. Archive when complete
/opsx:archive
```

### Avoid:
- ❌ `/fleet` mode (use longer single autopilot sessions instead)
- ❌ Creating parallel spec systems
- ❌ Adding heavy plugins/MCPs without clear ROI
- ❌ Long-lived local branches without regular merges

---

## Known Limitations

- **CUDA**: Environment in test phase lacks CUDA toolkit (builds with CPU-only mode)
- **OpenGL**: Full graphics examples require GL dev libraries
- **LSP**: Requires compile_commands.json from build/ (auto-generated)

---

## Contact & Archival

- **Repository**: https://github.com/LessUp/n-body
- **Pages**: https://lessup.github.io/n-body/
- **Maintainer**: See CONTRIBUTING.md

---

Last Updated: 2024-04-24
Status: ✅ Ready for Archive
