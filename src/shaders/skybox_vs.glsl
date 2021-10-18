#version 330 core

layout (location = 0) in vec3 AttribPosition;

out vec3 FragPosition;

uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPosition = AttribPosition;

    vec4 position = projection * mat4(mat3(view)) * vec4(AttribPosition, 1.0);
    gl_Position = position.xyww;
}  