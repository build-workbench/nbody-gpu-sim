#include "nbody/ui_panel.hpp"

#if NBODY_WITH_RENDERING && NBODY_WITH_UI

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace nbody {

UIPanel::UIPanel() = default;

UIPanel::~UIPanel() {
  if (initialized_) {
    cleanup();
  }
}

void UIPanel::initialize() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // Setup style
  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 5.0f;
  style.FrameRounding = 3.0f;

  // Initialize backends
  ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
  ImGui_ImplOpenGL3_Init("#version 330");

  initialized_ = true;
}

void UIPanel::cleanup() {
  if (initialized_) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
  }
}

void UIPanel::newFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void UIPanel::render() {
  if (!visible_) {
    return;
  }

  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);

  ImGui::Begin("Diagnostics", &visible_,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

  // Performance section
  ImGui::Text("Performance");
  ImGui::Indent();
  ImGui::Text("FPS: %.1f", fps_);
  ImGui::Text("Frame: %.2f ms", frame_time_);
  ImGui::Unindent();

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Simulation section
  ImGui::Text("Simulation");
  ImGui::Indent();
  ImGui::Text("Particles: %zu", particle_count_);
  ImGui::Text("Method: %s", forceMethodToString(force_method_));
  ImGui::Text("Time: %.2f s", simulation_time_);
  ImGui::Unindent();

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Controls section
  ImGui::Text("Controls");

  // Pause/Resume button
  if (ImGui::Button(paused_ ? "Resume" : "Pause", ImVec2(80, 0))) {
    paused_ = !paused_;
  }
  ImGui::SameLine();

  // Reset button
  if (ImGui::Button("Reset", ImVec2(80, 0))) {
    reset_requested_ = true;
  }

  ImGui::Spacing();

  // Method selection
  ImGui::Text("Force Method:");
  const char* methods[] = {"Direct N2", "Barnes-Hut", "Spatial Hash"};
  int current = static_cast<int>(selected_method_);
  if (ImGui::Combo("##Method", &current, methods, IM_ARRAYSIZE(methods))) {
    selected_method_ = static_cast<ForceMethod>(current);
    method_changed_ = true;
  }

  ImGui::Spacing();
  ImGui::TextDisabled("Press F1 to toggle");

  ImGui::End();
}

void UIPanel::endFrame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

const char* UIPanel::forceMethodToString(ForceMethod method) {
  switch (method) {
  case ForceMethod::DIRECT_N2:
    return "Direct N2";
  case ForceMethod::BARNES_HUT:
    return "Barnes-Hut";
  case ForceMethod::SPATIAL_HASH:
    return "Spatial Hash";
  default:
    return "Unknown";
  }
}

}  // namespace nbody

#endif  // NBODY_WITH_RENDERING && NBODY_WITH_UI
