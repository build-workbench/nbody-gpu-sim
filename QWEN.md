# QWEN.md — N-Body Particle Simulation Project Context

## Project Overview

**N-Body Particle Simulation** is a high-performance, GPU-accelerated physics engine capable of simulating up to 1 million particles in real-time. Built with **CUDA C++17** and **OpenGL 3.3+**, it implements three force-calculation algorithms — Direct N², Barnes-Hut O(N log N), and Spatial Hash O(N) — running entirely on the GPU with zero-copy CUDA-OpenGL interop rendering.

### Key Technologies
- **Language**: C++17 with CUDA 11.0+
- **Rendering**: OpenGL 3.3+, GLFW, GLEW, GLM
- **Build System**: CMake 3.18+
- **Testing**: Google Test + RapidCheck (property-based)
- **GPU**: NVIDIA Compute Capability 7.5+ (Turing → Hopper)

### Architecture Highlights
- **Strategy Pattern**: Force calculators for runtime algorithm switching
- **Bridge Pattern**: CUDA-OpenGL interop for zero-copy rendering
- **Facade Pattern**: ParticleSystem API for simulation control
- **SoA Layout**: Structure of Arrays for GPU memory coalescing
- **Velocity Verlet**: Symplectic integration for energy conservation

---

## Directory Structure

```
n-body/
├── specs/                    # Spec documents (Single Source of Truth)
│   ├── product/              # Product requirements & acceptance criteria
│   └── rfc/                  # Technical design documents
├── docs/                     # User-facing documentation
│   ├── setup/                # Setup guides
│   ├── tutorials/            # Tutorials and usage examples
│   ├── architecture/         # Architecture documentation
│   ├── assets/               # Images and diagrams
│   └── zh-CN/                # Chinese translations
├── site/                     # GitHub Pages site files
│   ├── _config.yml           # Jekyll configuration
│   ├── index.md              # Site entry point
│   └── Gemfile               # Ruby dependencies
├── changelog/                # Version changelog (en/zh-CN)
├── include/nbody/            # Public headers
├── src/                      # Implementation
│   ├── core/                 # Core simulation logic
│   ├── cuda/                 # CUDA kernels
│   ├── render/               # OpenGL rendering
│   └── utils/                # Utility functions
├── tests/                    # Test files (Google Test + RapidCheck)
├── examples/                 # Usage examples
├── scripts/                  # Build and automation scripts
├── .github/                  # GitHub workflows (CI, Pages)
├── .vscode/                  # VS Code settings
├── build/                    # Build artifacts (gitignored)
├── CMakeLists.txt            # Main build configuration
├── AGENTS.md                 # AI agent instructions (Spec-Driven Development)
├── CONTRIBUTING.md           # Contribution guidelines
└── README.md                 # Project overview
```

---

## Building and Running

### Prerequisites
| Component | Minimum | Recommended |
|-----------|---------|-------------|
| GPU | NVIDIA CC 7.0+ | NVIDIA CC 8.0+ (RTX 3000+) |
| CUDA | 11.0 | 12.0+ |
| CMake | 3.18 | 3.25+ |
| OpenGL | 3.3 | 4.5+ |

### Build Commands

```bash
# Option 1: Using build script
./scripts/build.sh

# Option 2: Manual build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Run Simulation

```bash
./build/nbody_sim 100000    # 100K particles
./build/nbody_sim 1000000   # 1 million particles
./build/nbody_sim           # Default: 10K particles
```

### Testing

```bash
# Run all tests
./scripts/test.sh

# Or manually
cd build && ./nbody_tests

# Run specific test suite
./nbody_tests --gtest_filter=ForceCalculation.*

# Verbose output
./nbody_tests --gtest_color=yes --gtest_brief=0
```

### Code Formatting

```bash
# Format all source files
./scripts/format.sh

# Or manually
clang-format -i src/**/*.cpp src/**/*.cu include/**/*.hpp
```

---

## Development Conventions

### Spec-Driven Development (SDD)

This project strictly follows **Spec-Driven Development**. The `/specs` directory is the **Single Source of Truth**:

1. **Review specs first** before writing any code
2. **Update specs** before implementing new features or changing interfaces
3. **Implement exactly to spec** — no gold-plating
4. **Test against spec** acceptance criteria

Key spec files:
- `specs/product/n-body-simulation.md` — Product requirements
- `specs/rfc/0001-core-architecture.md` — Technical design decisions

### Code Style

- **C++/CUDA**: Formatted via **clang-format** (see `.clang-format`)
- **Editor settings**: Defined in `.editorconfig`
- **Markdown**: Linted via `.markdownlint.json`

### Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

| Prefix | Description |
|--------|-------------|
| `feat:` | New feature |
| `fix:` | Bug fix |
| `docs:` | Documentation update |
| `perf:` | Performance improvement |
| `refactor:` | Code refactoring |
| `test:` | Testing related |
| `chore:` | Build or tooling changes |

### Adding New Force Methods

1. Create class inheriting from `ForceCalculator`
2. Implement `computeForces(ParticleData*)`
3. Add enum value to `ForceMethod`
4. Register in `createForceCalculator()` factory
5. Update specs if interface changes

### Testing Practices

- **Unit tests**: Individual component verification
- **Property-based tests**: Automatic test generation via RapidCheck
- **Integration tests**: Full pipeline and performance benchmarks
- Target: >90% test coverage for core components

---

## Key Implementation Details

### Particle Data (SoA Layout)

Memory per particle: **13 floats = 52 bytes**
- Position: pos_x, pos_y, pos_z
- Velocity: vel_x, vel_y, vel_z
- Acceleration: acc_x, acc_y, acc_z (current + old)
- Mass: mass

### Force Algorithms

| Algorithm | Time Complexity | Space Complexity | Best Use Case |
|-----------|----------------|-----------------|---------------|
| Direct N² | O(N²) | O(1) | Small systems, precision validation |
| Barnes-Hut | O(N log N) | O(N) | Large-scale gravitational simulation |
| Spatial Hash | O(N) | O(N) | Short-range forces, molecular dynamics |

### Performance Targets (RTX 3080)

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|:---------:|:---------:|:----------:|:------------:|
| 10K | 60 FPS | 120+ FPS | 120+ FPS |
| 100K | ~10 FPS | 60+ FPS | 90+ FPS |
| 1M | <1 FPS | 25+ FPS | 60+ FPS |

### Simulation Controls

| Key | Action |
|-----|--------|
| `Space` | Pause/Resume |
| `1/2/3` | Switch: Direct N² → Barnes-Hut → Spatial Hash |
| `R` | Reset simulation |
| `C` | Reset camera |
| `Esc` | Exit |
| Mouse drag | Rotate view |
| Scroll wheel | Zoom |

---

## CI/CD

### GitHub Workflows

- **CI** (`.github/workflows/ci.yml`): Format check, documentation consistency, markdown lint, build info
- **Pages** (`.github/workflows/pages.yml`): Build and deploy GitHub Pages site

### Workflow Triggers

- **CI**: Push/PR to main/master on C++/CUDA/CMake changes
- **Pages**: Push/PR on markdown, docs, changelog, examples, or site changes

---

## References

- [CONTRIBUTING.md](CONTRIBUTING.md) — Detailed contribution guidelines
- [Product Spec](specs/product/n-body-simulation.md) — Requirements and acceptance criteria
- [Architecture RFC](specs/rfc/0001-core-architecture.md) — Technical design decisions
- [AGENTS.md](AGENTS.md) — AI agent workflow instructions
