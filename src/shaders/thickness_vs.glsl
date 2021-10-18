#version 330 core

layout (location = 0) in vec3 AttribPosition;
layout (location = 3) in vec3 ParticlePosition;

out vec3 CenterPositionViewSpace;
out vec3 FragPositionViewSpace;

uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 model = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(ParticlePosition, 1.0));

    CenterPositionViewSpace = vec3(view * vec4(ParticlePosition, 1.0));
    FragPositionViewSpace = vec3(view * model * vec4(AttribPosition, 1.0));

    gl_Position = projection * vec4(FragPositionViewSpace, 1.0);
}