#version 330 core
in float depth;
out vec4 FragColor;

uniform float maxDepth;
uniform float maxVelocity;
uniform int colorMode;

void main() {
    // Circular point sprite
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;
    
    // Color based on mode
    vec3 color;
    float t = clamp(depth / maxDepth, 0.0, 1.0);
    
    if (colorMode == 0) {
        // Depth-based coloring (warm to cool)
        color = mix(vec3(1.0, 0.5, 0.0), vec3(0.0, 0.5, 1.0), t);
    } else if (colorMode == 1) {
        // Velocity-based
        color = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), t);
    } else {
        // Density-based
        color = mix(vec3(0.2, 0.2, 0.8), vec3(1.0, 1.0, 0.2), t);
    }
    
    // Edge softening
    float alpha = 1.0 - smoothstep(0.4, 0.5, dist);
    
    FragColor = vec4(color, alpha);
}
