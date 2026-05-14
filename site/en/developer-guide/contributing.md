# Contributing

How to contribute to the project.

## Development Setup

```bash
git clone https://github.com/AICL-Lab/n-body.git
cd n-body
./scripts/build.sh
./scripts/test.sh
```

## Code Style

- Use clang-format (see `.clang-format`)
- Follow C++20 best practices
- Document public APIs

## Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Make changes and add tests
4. Run tests: `./scripts/test.sh`
5. Format code: `clang-format -i src/*.cpp`
6. Submit PR

## Adding New Features

1. Update OpenSpec specs in `openspec/specs/`
2. Implement the feature
3. Add tests
4. Update documentation

## Reporting Issues

Use GitHub Issues with:
- Description of the problem
- Steps to reproduce
- Expected vs actual behavior
- System information
