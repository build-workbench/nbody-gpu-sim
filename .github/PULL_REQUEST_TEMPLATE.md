## Description

<!-- Provide a clear and concise description of the changes -->

### Type of Change

- [ ] 🐛 Bug fix (non-breaking change which fixes an issue)
- [ ] ✨ New feature (non-breaking change which adds functionality)
- [ ] 💥 Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] 📝 Documentation update
- [ ] 🎨 Style/formatting change
- [ ] ♻️ Code refactoring
- [ ] ⚡ Performance improvement
- [ ] ✅ Test addition/update

### Related Issues

<!-- Link to related issues: Fixes #123, Closes #456 -->

## Changes Made

<!-- List the key changes made -->

1.
2.
3.

## Testing

### Test Environment

- OS:
- CUDA Version:
- GPU:

### Tests Run

```bash
# Commands used to test
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
ctest --output-on-failure
```

### Test Results

- [ ] All existing tests pass
- [ ] New tests added for new functionality
- [ ] Manual testing performed

### Performance Impact

<!-- If applicable, describe any performance impact -->

| Metric | Before | After |
|--------|--------|-------|
| FPS (N=100K) | | |
| Memory | | |

## Checklist

- [ ] Code follows the project's style guidelines
- [ ] Code has been self-reviewed
- [ ] Code has been commented, particularly in hard-to-understand areas
- [ ] Corresponding changes to documentation made
- [ ] No new warnings introduced
- [ ] Tests added that prove the fix is effective or feature works
- [ ] New and existing unit tests pass locally
- [ ] Any dependent changes have been merged and published

## Documentation Updates

<!-- List any documentation that needs updating -->

- [ ] README.md
- [ ] CHANGELOG.md
- [ ] docs/
- [ ] API documentation

## Additional Notes

<!-- Add any additional notes or context about the pull request -->

## Screenshots (if applicable)

<!-- Add screenshots to help explain the changes -->
