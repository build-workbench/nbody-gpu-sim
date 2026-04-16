# AGENTS.md — AI Agent Workflow Instructions

## Project Philosophy: Spec-Driven Development (SDD)

This project strictly follows the **Spec-Driven Development (SDD)** paradigm. All code implementations must use the `/specs` directory as the Single Source of Truth.

---

## Directory Context

| Path | Purpose |
|------|---------|
| `/specs/product/` | Product feature definitions, requirements, and acceptance criteria |
| `/specs/rfc/` | Technical design documents and architecture decisions |
| `/specs/api/` | API interface definitions (e.g., OpenAPI, GraphQL schemas) |
| `/specs/db/` | Database schema definitions and conventions |
| `/specs/testing/` | BDD test case specifications and acceptance test suites |
| `/docs/` | User-facing documentation (setup guides, tutorials, architecture guides) |
| `/docs/zh-CN/` | Chinese translations of user-facing documentation |

---

## AI Agent Workflow Instructions

When you (the AI agent) are asked to develop a new feature, modify an existing feature, or fix a bug, **you must strictly follow this workflow without skipping any steps**:

### Step 1: Review Specs

- Before writing any code, first read the relevant product spec, RFC, and API definition in `/specs/`.
- If the user's request conflicts with an existing spec, **stop immediately** and point out the conflict. Ask the user whether to update the spec first.

### Step 2: Spec-First Update

- If this is a new feature, or if existing interfaces/database structures need to change, **you must first propose modifications to the relevant spec documents** (e.g., `/specs/product/`, `/specs/rfc/`, `/specs/api/`).
- Wait for user confirmation on the spec changes before proceeding to the code implementation phase.

### Step 3: Implementation

- When writing code, **strictly follow 100% of the definitions in the specs** (including variable naming, API paths, data types, status codes, etc.).
- **Do not add features not defined in the specs** (No Gold-Plating).

### Step 4: Test Against Spec

- Write unit and integration tests based on the acceptance criteria in `/specs/`.
- Ensure test cases cover all boundary conditions described in the specs.

---

## Code Generation Rules

- Any externally exposed API changes **must** be accompanied by corresponding updates to `/specs/api/`.
- If uncertain about technical details, consult `/specs/rfc/` for architectural conventions. **Do not invent design patterns on your own.**
- When generating a Pull Request, reference the relevant spec files and describe how the implementation satisfies each requirement.

---

## Quick Reference

### Spec Files

| Spec | Path |
|------|------|
| Product Requirements | `specs/product/n-body-simulation.md` |
| Core Architecture | `specs/rfc/0001-core-architecture.md` |

### Key Conventions

- **C++17** with **CUDA 11.0+**
- **Strategy Pattern** for force calculators
- **SoA (Structure of Arrays)** for GPU particle data
- **Velocity Verlet** integration scheme
- **Zero-copy** CUDA-OpenGL interop for rendering
- Code formatting via **clang-format** (see `.clang-format`)
- Commit messages follow **Conventional Commits**
