#pragma once

#include "nbody/types.hpp"
#include <string>

namespace nbody {

/**
 * @class UIPanel
 * @brief Dear ImGui-based diagnostics panel for runtime inspection and tuning
 *
 * Provides an in-application panel for viewing performance metrics,
 * simulation state, and adjusting parameters at runtime.
 *
 * This class is only available when both NBODY_WITH_RENDERING and
 * NBODY_WITH_UI are enabled.
 */
class UIPanel {
public:
  UIPanel();
  ~UIPanel();

  // Non-copyable (owns ImGui context)
  UIPanel(const UIPanel&) = delete;
  UIPanel& operator=(const UIPanel&) = delete;

  /// Initialize ImGui context and backends (call after OpenGL/GLEW init)
  void initialize();

  /// Cleanup ImGui context and backends
  void cleanup();

  /// Begin new ImGui frame (call at start of render loop)
  void newFrame();

  /// Render the diagnostics panel content
  void render();

  /// End frame and draw (call before glfwSwapBuffers)
  void endFrame();

  // ── State setters (called from application) ─────────────────────

  void setFps(float fps) { fps_ = fps; }
  void setFrameTime(float ms) { frame_time_ = ms; }
  void setParticleCount(size_t count) { particle_count_ = count; }
  void setSimulationTime(float time) { simulation_time_ = time; }
  void setForceMethod(ForceMethod method) { force_method_ = method; }
  void setPaused(bool paused) { paused_ = paused; }

  // ── State getters for callbacks ─────────────────────────────────

  bool isPaused() const { return paused_; }
  bool shouldReset() const { return reset_requested_; }
  void clearResetFlag() { reset_requested_ = false; }

  ForceMethod getSelectedMethod() const { return selected_method_; }
  bool methodChanged() const { return method_changed_; }
  void clearMethodChangedFlag() { method_changed_ = false; }

  /// Toggle panel visibility
  void toggleVisibility() { visible_ = !visible_; }
  bool isVisible() const { return visible_; }

private:
  bool initialized_ = false;
  bool visible_ = true;

  // Display state
  float fps_ = 0.0f;
  float frame_time_ = 0.0f;
  size_t particle_count_ = 0;
  float simulation_time_ = 0.0f;
  ForceMethod force_method_ = ForceMethod::DIRECT_N2;
  bool paused_ = false;

  // User input state
  bool reset_requested_ = false;
  ForceMethod selected_method_ = ForceMethod::DIRECT_N2;
  bool method_changed_ = false;

  // Helper to convert ForceMethod to string
  static const char* forceMethodToString(ForceMethod method);
};

}  // namespace nbody
