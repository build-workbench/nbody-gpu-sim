# Capability: Repository Governance

## Overview

Defines the repository operating model for OpenSpec-driven development, documentation quality, AI instruction files, automation hygiene, GitHub presentation, and final closeout workflow.

---

## ADDED Requirements

### Requirement: OpenSpec Source of Truth

The repository SHALL treat `openspec/` as the only active specification system for new development, maintenance, and closeout work.

#### Scenario: Active specifications are unambiguous
- **WHEN** a contributor or AI agent looks for the current requirements
- **THEN** the canonical source SHALL be `openspec/specs/` and any legacy spec trees SHALL be marked as historical-only references

#### Scenario: Change work starts from OpenSpec
- **WHEN** a feature, fix, refactor, or closeout task changes repository behavior or workflow
- **THEN** the work SHALL start from an OpenSpec proposal or an existing active OpenSpec change before implementation proceeds

---

### Requirement: Repository Structure Governance

The repository SHALL maintain a deliberate structure in which documentation, site content, changelog content, and engineering configuration have distinct purposes and do not drift into redundant mirrors.

#### Scenario: Duplicate content is eliminated
- **WHEN** two files or directories serve the same audience and purpose
- **THEN** one canonical surface SHALL remain and the duplicate SHALL be removed, redirected, or reduced to a pointer

#### Scenario: Historical material is clearly separated
- **WHEN** older structure summaries, legacy specs, or superseded docs are retained
- **THEN** they SHALL be labeled as historical or legacy content and SHALL NOT present themselves as the active workflow

---

### Requirement: AI Collaboration Instructions

The repository SHALL provide project-specific AI instruction files that agree on workflow, repository structure, and engineering expectations.

#### Scenario: Instruction files stay aligned
- **WHEN** `AGENTS.md`, `CLAUDE.md`, or Copilot instruction files describe the development process
- **THEN** they SHALL all point to the same OpenSpec-first workflow and SHALL NOT reference contradictory repository structures

#### Scenario: Generic boilerplate is rejected
- **WHEN** an AI instruction file is added or updated
- **THEN** it SHALL contain project-specific guidance that materially helps work on this repository rather than generic assistant boilerplate

---

### Requirement: Workflow Automation Quality

The repository SHALL keep only meaningful automation and SHALL scope workflow triggers to changes that justify execution.

#### Scenario: Workflows validate real repository paths
- **WHEN** CI or Pages workflows execute
- **THEN** they SHALL validate the documented build, test, docs, or site pipeline instead of relying only on ceremonial string checks

#### Scenario: Workflow noise is minimized
- **WHEN** repository changes do not affect a workflow's owned surface
- **THEN** that workflow SHALL NOT trigger

---

### Requirement: Developer Environment Baseline

The repository SHALL document and support a lean developer tooling baseline centered on the canonical build path and reusable language tooling.

#### Scenario: LSP setup follows the build system
- **WHEN** contributors configure editor assistance
- **THEN** the preferred C++/CUDA LSP path SHALL derive from the primary CMake-generated compile database

#### Scenario: Local guardrails are available
- **WHEN** contributors prepare changes locally
- **THEN** the repository SHALL provide lightweight hooks or equivalent automation for the highest-value quality checks

---

### Requirement: GitHub Project Presentation

The repository SHALL present itself clearly on GitHub and GitHub Pages so that new users can quickly understand what the project is, why it matters, and where to go next.

#### Scenario: Repository metadata is curated
- **WHEN** a user views the repository About section
- **THEN** the description, topics, and homepage link SHALL accurately reflect the project and include the published Pages URL when available

#### Scenario: Pages acts as a showcase
- **WHEN** a user visits GitHub Pages
- **THEN** the landing page SHALL explain the project value proposition, highlight the technical differentiators, and route users to documentation, examples, and source code

---

### Requirement: Closeout Operating Model

The repository SHALL document a lightweight closeout workflow that supports final stabilization work without introducing unnecessary branching or tool overhead.

#### Scenario: Long-running implementation sessions are preferred
- **WHEN** AI-assisted implementation is planned for a broad cleanup or closeout task
- **THEN** the documented recommendation SHALL prefer a longer `autopilot` execution over routine `/fleet` usage unless the work is genuinely parallel-heavy

#### Scenario: Review gates remain explicit
- **WHEN** a major structural or governance refactor is prepared for merge
- **THEN** the workflow SHALL require a deliberate review step such as `/review` before finalization
