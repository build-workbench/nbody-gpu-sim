# AGENTS.md — AI Agent Workflow Instructions

## Project Philosophy: Spec-Driven Development (SDD)

This project strictly follows the **Spec-Driven Development (SDD)** paradigm. All code implementations must use the `/specs` directory as the **Single Source of Truth**.

---

## Directory Context

| Path | Purpose |
|------|---------|
| `/specs/product/` | Product feature definitions, requirements, and acceptance criteria |
| `/specs/rfc/` | Technical design documents and architecture decisions |
| `/docs/` | User-facing documentation (setup guides, tutorials, architecture guides) |
| `/docs/zh-CN/` | Chinese translations of user-facing documentation |
| `/examples/` | Code examples for common use cases |

---

## AI Agent Workflow Instructions

When you (the AI agent) are asked to develop a new feature, modify an existing feature, or fix a bug, **you must strictly follow this workflow without skipping any steps**:

### Step 1: Review Specs

- Before writing any code, first read the relevant product spec and RFC in `/specs/`.
- If the user's request conflicts with an existing spec, **stop immediately** and point out the conflict. Ask the user whether to update the spec first.

### Step 2: Spec-First Update

- If this is a new feature, or if existing interfaces/architecture need to change, **you must first propose modifications to the relevant spec documents** (e.g., `/specs/product/`, `/specs/rfc/`).
- Wait for user confirmation on the spec changes before proceeding to the code implementation phase.

### Step 3: Implementation

- When writing code, **strictly follow 100% of the definitions in the specs** (including variable naming, API paths, data types, status codes, etc.).
- **Do not add features not defined in the specs** (No Gold-Plating).
- Maintain consistency with existing code style and patterns.

### Step 4: Test Against Spec

- Write unit and integration tests based on the acceptance criteria in `/specs/`.
- Ensure test cases cover all boundary conditions described in the specs.
- Run existing tests to ensure no regressions.

---

## Code Generation Rules

- Any externally exposed API changes **must** be accompanied by corresponding updates to specs.
- If uncertain about technical details, consult `/specs/rfc/` for architectural conventions. **Do not invent design patterns on your own.**
- When generating a Pull Request, reference the relevant spec files and describe how the implementation satisfies each requirement.

---

## Spec Files

| Spec | Path | Description |
|------|------|-------------|
| Product Requirements | `specs/product/n-body-simulation.md` | Functional requirements, acceptance criteria, traceability matrix |
| Core Architecture | `specs/rfc/0001-core-architecture.md` | System design, components, algorithms, correctness properties |

---

## Key Conventions

### Technology Stack
- **C++17** with **CUDA 11.0+**
- **OpenGL 3.3+** for rendering
- **CMake 3.18+** for build system

### Design Patterns
- **Strategy Pattern** for force calculators (runtime algorithm switching)
- **Bridge Pattern** for CUDA-OpenGL interop
- **Facade Pattern** for ParticleSystem API

### Data Structures
- **SoA (Structure of Arrays)** for GPU particle data (memory coalescing)
- **Velocity Verlet** symplectic integration (energy conservation)

### Code Style
- Formatting via **clang-format** (see `.clang-format`)
- Editor settings via `.editorconfig`
- Commit messages follow **Conventional Commits**

---

## Project Structure

```
n-body/
├── specs/                    # Spec documents (Single Source of Truth)
│   ├── product/              # Product requirements
│   │   └── n-body-simulation.md
│   └── rfc/                  # Technical design documents
│       └── 0001-core-architecture.md
├── docs/                     # User documentation
│   ├── setup/                # Setup guides
│   ├── architecture/         # Architecture documentation
│   └── zh-CN/                # Chinese translations
├── include/nbody/            # Public headers
├── src/                      # Implementation
│   ├── core/                 # Core simulation logic
│   ├── cuda/                 # CUDA kernels
│   ├── render/               # OpenGL rendering
│   └── utils/                # Utility functions
├── tests/                    # Test files
├── examples/                 # Usage examples
├── AGENTS.md                 # This file - AI agent instructions
├── CONTRIBUTING.md           # Contribution guidelines
├── README.md                 # Project overview (English)
└── README.zh-CN.md           # Project overview (Chinese)
```

---

## Testing

```bash
cd build
./nbody_tests                           # Run all tests
./nbody_tests --gtest_filter=ForceCalculation.*  # Specific suite
```

---

## Common Tasks

### Adding a New Force Method

1. Create class inheriting from `ForceCalculator`
2. Implement `computeForces(ParticleData*)`
3. Add enum value to `ForceMethod`
4. Register in `createForceCalculator()` factory
5. Update specs if interface changes

### Adding a New Particle Distribution

1. Create parameter struct
2. Implement initialization kernel
3. Add enum value to `InitDistribution`
4. Update product spec with new requirement

### Performance Optimization

1. Profile with Nsight Compute/Systems
2. Identify bottlenecks (memory bandwidth, compute, etc.)
3. Consult architecture RFC for optimization guidelines
4. Ensure changes don't violate correctness properties

---

## References

- [CONTRIBUTING.md](CONTRIBUTING.md) — Contribution guidelines
- [Product Spec](specs/product/n-body-simulation.md) — Requirements and acceptance criteria
- [Architecture RFC](specs/rfc/0001-core-architecture.md) — Technical design decisions
