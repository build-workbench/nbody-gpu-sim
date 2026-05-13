# Renderer API

OpenGL visualization.

## Class

```cpp
namespace nbody {

class Renderer {
public:
    void initialize(int width, int height);
    void render(const ParticleData& data);
    void resize(int width, int height);
    
    void setPointSize(float size);
    void setParticleColor(const glm::vec3& color);
    
    Camera& getCamera();
};

} // namespace nbody
```

## Usage

```cpp
Renderer renderer;
renderer.initialize(1280, 720);
renderer.setPointSize(2.0f);

while (running) {
    renderer.render(system.getParticleData());
}
```
