#include "nbody/app_cli.hpp"
#include "nbody/error_handling.hpp"
#include "nbody/particle_system.hpp"
#include "nbody/performance_observability.hpp"
#include "nbody/renderer.hpp"

#if NBODY_WITH_UI
#include "nbody/ui_panel.hpp"
#endif

#include <GLFW/glfw3.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace nbody;

// ============================================================================
// Application Class
// ============================================================================
// Encapsulates all application state and callbacks, avoiding global mutable
// state. The GLFW user pointer mechanism is used to pass this to callbacks.

class Application {
public:
  // Configuration constants
  static constexpr int DEFAULT_WINDOW_WIDTH = 1280;
  static constexpr int DEFAULT_WINDOW_HEIGHT = 720;
  static constexpr float DEFAULT_MOUSE_SENSITIVITY = 0.005f;
  static constexpr float DEFAULT_ZOOM_SENSITIVITY = 2.0f;

  Application() = default;
  ~Application() = default;

  // Non-copyable, non-movable (owns resources)
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;

  int run(int argc, char* argv[]) {
    try {
      options_ = parseAppCliOptions(argc, const_cast<const char* const*>(argv));
      if (options_.show_help) {
        std::cout << appCliUsage();
        return 0;
      }

      particle_count_ = options_.particle_count;

      if (options_.benchmark_mode) {
        return runBenchmarkMode();
      }

      std::cout << "N-Body Particle Simulation\n";
      std::cout << "Particle count: " << particle_count_ << "\n";
      printControls();

      initialize();
      mainLoop();
      cleanup();

    } catch (const CudaException& e) {
      std::cerr << "CUDA Error: " << e.what() << std::endl;
      return 1;
    } catch (const OpenGLException& e) {
      std::cerr << "OpenGL Error: " << e.what() << std::endl;
      return 1;
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }

    return 0;
  }

private:
  // State
  GLFWwindow* window_ = nullptr;
  ParticleSystem particle_system_;
  Renderer renderer_;
  size_t particle_count_ = 10000;
  bool paused_ = false;
  AppCliOptions options_;
  float mouse_sensitivity_ = DEFAULT_MOUSE_SENSITIVITY;
  float zoom_sensitivity_ = DEFAULT_ZOOM_SENSITIVITY;
  double last_mouse_x_ = 0;
  double last_mouse_y_ = 0;
  bool mouse_pressed_ = false;

#if NBODY_WITH_UI
  UIPanel ui_panel_;
#endif

  void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW");
    }

    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "N-Body Simulation",
                               nullptr, nullptr);
    if (!window_) {
      glfwTerminate();
      throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);  // VSync

    // Store 'this' in GLFW window user pointer for callbacks
    glfwSetWindowUserPointer(window_, this);

    // Set callbacks (static wrapper functions)
    glfwSetKeyCallback(window_, keyCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetCursorPosCallback(window_, cursorPosCallback);
    glfwSetScrollCallback(window_, scrollCallback);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

    // Initialize renderer
    renderer_.initialize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

    // Initialize particle system
    SimulationConfig config;
    config.particle_count = options_.particle_count;
    config.init_distribution = InitDistribution::SPHERICAL;
    config.force_method = options_.force_method;
    config.dt = options_.dt;
    config.G = options_.G;
    config.softening = options_.softening;
    config.barnes_hut_theta = options_.barnes_hut_theta;
    config.spatial_hash_cell_size = options_.spatial_hash_cell_size;
    config.spatial_hash_cutoff = options_.spatial_hash_cutoff;

    particle_system_.initialize(config);
    particle_system_.initializeInterop();
    paused_ = false;

#if NBODY_WITH_UI
    ui_panel_.initialize();
#endif
  }

  void mainLoop() {
    auto last_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;
    float fps = 0.0f;

    while (!glfwWindowShouldClose(window_)) {
      auto current_time = std::chrono::high_resolution_clock::now();
      float delta_time = std::chrono::duration<float>(current_time - last_time).count();

      // Update FPS counter
      frame_count++;
      if (delta_time >= 1.0f) {
        fps = frame_count / delta_time;
        frame_count = 0;
        last_time = current_time;

        // Update window title
        std::ostringstream title;
        title << "N-Body Simulation | " << particle_count_ << " particles | " << std::fixed
              << std::setprecision(1) << fps << " FPS | " << "Time: " << std::setprecision(2)
              << particle_system_.getSimulationTime();
        glfwSetWindowTitle(window_, title.str().c_str());
      }

#if NBODY_WITH_UI
      ui_panel_.newFrame();
#endif

      // Update simulation
      if (!paused_) {
        particle_system_.update(particle_system_.getTimeStep());
      }

      // Render with velocity VBO for velocity-based coloring
      renderer_.render(particle_system_.getInterop()->getPositionVBO(),
                       particle_system_.getParticleCount(),
                       particle_system_.getInterop()->getVelocityVBO());

#if NBODY_WITH_UI
      // Update UI state
      ui_panel_.setFps(fps);
      ui_panel_.setFrameTime(delta_time * 1000.0f);
      ui_panel_.setParticleCount(particle_system_.getParticleCount());
      ui_panel_.setSimulationTime(particle_system_.getSimulationTime());
      ui_panel_.setForceMethod(particle_system_.getForceMethod());
      ui_panel_.setPaused(paused_);

      // Render UI panel
      ui_panel_.render();
      ui_panel_.endFrame();

      // Handle UI callbacks
      if (ui_panel_.shouldReset()) {
        particle_system_.reset();
        ui_panel_.clearResetFlag();
      }
      if (ui_panel_.methodChanged()) {
        particle_system_.setForceMethod(ui_panel_.getSelectedMethod());
        ui_panel_.clearMethodChangedFlag();
      }
      paused_ = ui_panel_.isPaused();
#endif

      glfwSwapBuffers(window_);
      glfwPollEvents();
    }
  }

  void cleanup() {
#if NBODY_WITH_UI
    ui_panel_.cleanup();
#endif
    renderer_.cleanup();
    glfwDestroyWindow(window_);
    glfwTerminate();
  }

  // Static callback wrappers that dispatch to instance methods
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app)
      app->onKey(key, action);
  }

  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app)
      app->onMouseButton(button, action, window);
  }

  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app)
      app->onCursorPos(xpos, ypos);
  }

  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app)
      app->onScroll(yoffset);
  }

  static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app)
      app->onResize(width, height);
  }

  // Instance callback handlers
  void onKey(int key, int action) {
    if (action != GLFW_PRESS)
      return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window_, GLFW_TRUE);
      break;
    case GLFW_KEY_SPACE:
      paused_ = !paused_;
      if (paused_) {
        particle_system_.pause();
      } else {
        particle_system_.resume();
      }
      break;
    case GLFW_KEY_R:
      particle_system_.reset();
      break;
    case GLFW_KEY_1:
      particle_system_.setForceMethod(ForceMethod::DIRECT_N2);
      std::cout << "Switched to Direct N² method" << std::endl;
      break;
    case GLFW_KEY_2:
      particle_system_.setForceMethod(ForceMethod::BARNES_HUT);
      std::cout << "Switched to Barnes-Hut method" << std::endl;
      break;
    case GLFW_KEY_3:
      particle_system_.setForceMethod(ForceMethod::SPATIAL_HASH);
      std::cout << "Switched to Spatial Hash method" << std::endl;
      break;
    case GLFW_KEY_C:
      renderer_.getCamera().reset();
      break;
#if NBODY_WITH_UI
    case GLFW_KEY_F1:
      ui_panel_.toggleVisibility();
      break;
#endif
    }
  }

  void onMouseButton(int button, int action, GLFWwindow* window) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      mouse_pressed_ = (action == GLFW_PRESS);
      if (mouse_pressed_) {
        glfwGetCursorPos(window, &last_mouse_x_, &last_mouse_y_);
      }
    }
  }

  void onCursorPos(double xpos, double ypos) {
    if (mouse_pressed_) {
      double dx = xpos - last_mouse_x_;
      double dy = ypos - last_mouse_y_;

      renderer_.getCamera().orbit(static_cast<float>(-dx * mouse_sensitivity_),
                                  static_cast<float>(-dy * mouse_sensitivity_));

      last_mouse_x_ = xpos;
      last_mouse_y_ = ypos;
    }
  }

  void onScroll(double yoffset) {
    renderer_.getCamera().zoom(static_cast<float>(yoffset * zoom_sensitivity_));
  }

  void onResize(int width, int height) { renderer_.onResize(width, height); }

  int runBenchmarkMode() {
    SimulationConfig config;
    config.particle_count = options_.particle_count;
    config.init_distribution = InitDistribution::SPHERICAL;
    config.force_method = options_.force_method;
    config.dt = options_.dt;
    config.G = options_.G;
    config.softening = options_.softening;
    config.barnes_hut_theta = options_.barnes_hut_theta;
    config.spatial_hash_cell_size = options_.spatial_hash_cell_size;
    config.spatial_hash_cutoff = options_.spatial_hash_cutoff;

    particle_system_.initialize(config);
    consumeGlobalPhaseSnapshot();

    const auto start = std::chrono::steady_clock::now();
    for (size_t step = 0; step < options_.benchmark_steps; ++step) {
      particle_system_.update(particle_system_.getTimeStep());
    }
    const auto end = std::chrono::steady_clock::now();

    BenchmarkRunRecord record;
    record.benchmark_name = "application.benchmark_mode";
    record.force_method = config.force_method;
    record.particle_count = config.particle_count;
    record.iterations = options_.benchmark_steps;
    record.metrics["wall_time_ms"] = std::chrono::duration_cast<Milliseconds>(end - start).count() /
                                     static_cast<double>(options_.benchmark_steps);
    record.parameters["dt"] = config.dt;
    record.parameters["gravity"] = config.G;
    record.parameters["softening"] = config.softening;
    record.parameters["particle_count"] = static_cast<double>(config.particle_count);
    if (config.force_method == ForceMethod::BARNES_HUT) {
      record.parameters["theta"] = config.barnes_hut_theta;
    } else if (config.force_method == ForceMethod::SPATIAL_HASH) {
      record.parameters["cell_size"] = config.spatial_hash_cell_size;
      record.parameters["cutoff_radius"] = config.spatial_hash_cutoff;
    }
    record.phase_timings = consumeGlobalPhaseSnapshot();

    const std::vector<BenchmarkRunRecord> records{record};
    const std::string report = serializeBenchmarkRunRecords(records);
    std::cout << report << std::endl;
    if (!options_.benchmark_output_path.empty()) {
      writeBenchmarkRunRecords(options_.benchmark_output_path, records);
    }
    return 0;
  }

  static void printControls() {
    std::cout << "\nN-Body Simulation Controls:\n"
              << "  Space  - Pause/Resume\n"
              << "  R      - Reset simulation\n"
              << "  1      - Direct N² method\n"
              << "  2      - Barnes-Hut method\n"
              << "  3      - Spatial Hash method\n"
              << "  C      - Reset camera\n"
              << "  Mouse  - Rotate view\n"
              << "  Scroll - Zoom\n"
              << "  Esc    - Quit\n"
#if NBODY_WITH_UI
              << "  F1     - Toggle diagnostics panel\n"
#endif
              << "\n"
              << appCliUsage() << "\n";
  }
};

// ============================================================================
// Main Entry Point
// ============================================================================

int main(int argc, char* argv[]) {
  Application app;
  return app.run(argc, argv);
}
