#version 330 core

layout (location = 0) in vec3 AttribPosition;
layout (location = 3) in vec3 ParticlePosition;

out float FragPositionZViewSpace;

uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 model = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(ParticlePosition, 1.0));
    vec4 FragPositionViewSpace = view * model * vec4(AttribPosition, 1.0);
    FragPositionZViewSpace = FragPositionViewSpace.z;

    gl_Position = projection * FragPositionViewSpace;
}