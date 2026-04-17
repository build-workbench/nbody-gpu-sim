# Security Policy

## Supported Versions

The following versions of the N-Body Particle Simulation project are currently being supported with security updates:

| Version | Supported          |
| ------- | ------------------ |
| 2.0.x   | :white_check_mark: |
| 1.0.x   | :x:                |

## Reporting a Vulnerability

We take the security of the N-Body Particle Simulation project seriously. If you discover a security vulnerability, please report it responsibly.

### How to Report

1. **Do not** open a public GitHub issue for security vulnerabilities
2. Email security reports to the maintainer via GitHub's private vulnerability reporting feature
3. Include the following information:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if available)

### Response Timeline

- **Initial Response**: Within 48 hours
- **Triage**: Within 7 days
- **Fix Development**: Depends on severity
  - Critical: 1-3 days
  - High: 1-2 weeks
  - Medium/Low: Next release

### Security Scope

**In Scope:**
- Code execution vulnerabilities
- Memory corruption issues
- Input validation bypass
- Denial of service vulnerabilities

**Out of Scope:**
- Issues requiring physical access to the machine
- Social engineering attacks
- DoS requiring extreme resources
- Issues in dependencies (report to upstream)

## Security Best Practices

When using this project:

1. **Validate Input**: Always validate particle counts and configuration parameters
2. **Resource Limits**: Be aware of GPU memory limitations when running large simulations
3. **Trusted Data**: Only load simulation state files from trusted sources
4. **Regular Updates**: Keep your CUDA toolkit and GPU drivers updated

## Dependencies

This project uses the following external dependencies:

| Dependency | Purpose | Security Notes |
|------------|---------|----------------|
| CUDA Toolkit | GPU computation | Keep updated for security patches |
| GLFW | Window/input handling | Vendored, rarely changes |
| GLEW | OpenGL extension loading | Vendored, rarely changes |
| GLM | Math library | Header-only, no runtime issues |
| Google Test | Testing framework | Dev dependency only |
| RapidCheck | Property testing | Dev dependency only |

For dependency-related security issues, please also check upstream advisories.

---

Thank you for helping keep the N-Body Particle Simulation project secure!
