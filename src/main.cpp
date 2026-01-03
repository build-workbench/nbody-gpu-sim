#include "nbody/particle_system.hpp"
#include "nbody/renderer.hpp"
#include "nbody/error_handling.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace nbody;

// Global state for callbacks
struct AppState {
    ParticleSystem* particle_system = nullptr;
    Renderer* renderer = nullptr;
    bool running = true;
    bool paused = false;
    float mouse_sensitivity = 0.005f;
    float zoom_sensitivity = 2.0f;
    double last_mouse_x = 0;
    double last_mouse_y = 0;
    bool mouse_pressed = false;
};

AppState g_app_state;

// Callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    
    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE:
            g_app_state.paused = !g_app_state.paused;
            if (g_app_state.paused) {
                g_app_state.particle_system->pause();
            } else {
                g_app_state.particle_system->resume();
            }
            break;
        case GLFW_KEY_R:
            g_app_state.particle_system->reset();
            break;
        case GLFW_KEY_1:
            g_app_state.particle_system->setForceMethod(ForceMethod::DIRECT_N2);
            std::cout << "Switched to Direct N² method" << std::endl;
            break;
        case GLFW_KEY_2:
            g_app_state.particle_system->setForceMethod(ForceMethod::BARNES_HUT);
            std::cout << "Switched to Barnes-Hut method" << std::endl;
            break;
        case GLFW_KEY_3:
            g_app_state.particle_system->setForceMethod(ForceMethod::SPATIAL_HASH);
            std::cout << "Switched to Spatial Hash method" << std::endl;
            break;
        case GLFW_KEY_C:
            g_app_state.renderer->getCamera().reset();
            break;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_app_state.mouse_pressed = (action == GLFW_PRESS);
        if (g_app_state.mouse_pressed) {
            glfwGetCursorPos(window, &g_app_state.last_mouse_x, &g_app_state.last_mouse_y);
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (g_app_state.mouse_pressed) {
        double dx = xpos - g_app_state.last_mouse_x;
        double dy = ypos - g_app_state.last_mouse_y;
        
        g_app_state.renderer->getCamera().orbit(
            static_cast<float>(-dx * g_app_state.mouse_sensitivity),
            static_cast<float>(-dy * g_app_state.mouse_sensitivity)
        );
        
        g_app_state.last_mouse_x = xpos;
        g_app_state.last_mouse_y = ypos;
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    g_app_state.renderer->getCamera().zoom(
        static_cast<float>(yoffset * g_app_state.zoom_sensitivity)
    );
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    g_app_state.renderer->onResize(width, height);
}

void printUsage() {
    std::cout << "\nN-Body Simulation Controls:\n"
              << "  Space  - Pause/Resume\n"
              << "  R      - Reset simulation\n"
              << "  1      - Direct N² method\n"
              << "  2      - Barnes-Hut method\n"
              << "  3      - Spatial Hash method\n"
              << "  C      - Reset camera\n"
              << "  Mouse  - Rotate view\n"
              << "  Scroll - Zoom\n"
              << "  Esc    - Quit\n\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    size_t particle_count = 10000;
    if (argc > 1) {
        particle_count = std::stoul(argv[1]);
    }
    
    std::cout << "N-Body Particle Simulation\n";
    std::cout << "Particle count: " << particle_count << "\n";
    printUsage();
    
    try {
        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        
        // Create window
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        GLFWwindow* window = glfwCreateWindow(1280, 720, "N-Body Simulation", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }
        
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);  // VSync
        
        // Set callbacks
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        
        // Initialize renderer
        Renderer renderer;
        renderer.initialize(1280, 720);
        g_app_state.renderer = &renderer;
        
        // Initialize particle system
        SimulationConfig config;
        config.particle_count = particle_count;
        config.init_distribution = InitDistribution::SPHERICAL;
        config.force_method = ForceMethod::DIRECT_N2;
        config.dt = 0.001f;
        config.G = 1.0f;
        config.softening = 0.1f;
        
        ParticleSystem particle_system;
        particle_system.initialize(config);
        particle_system.initializeInterop();
        g_app_state.particle_system = &particle_system;
        
        // Main loop
        auto last_time = std::chrono::high_resolution_clock::now();
        int frame_count = 0;
        float fps = 0.0f;
        
        while (!glfwWindowShouldClose(window)) {
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
                title << "N-Body Simulation | "
                      << particle_count << " particles | "
                      << std::fixed << std::setprecision(1) << fps << " FPS | "
                      << "Time: " << std::setprecision(2) << particle_system.getSimulationTime();
                glfwSetWindowTitle(window, title.str().c_str());
            }
            
            // Update simulation
            if (!g_app_state.paused) {
                particle_system.update(config.dt);
            }
            
            // Render
            renderer.render(particle_system.getInterop()->getPositionVBO(),
                           particle_system.getParticleCount());
            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
        // Cleanup
        renderer.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
        
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
