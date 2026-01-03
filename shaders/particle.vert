#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

out float depth;

void main() {
    vec4 viewPos = view * vec4(aPos, 1.0);
    gl_Position = projection * viewPos;
    gl_PointSize = pointSize / max(-viewPos.z, 0.1);
    depth = -viewPos.z;
}
