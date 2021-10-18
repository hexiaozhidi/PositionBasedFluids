#version 330 core

layout (location = 0) in vec3 AttribPosition;
layout (location = 2) in vec2 AttribTextCoord;

out vec3 FragPosition;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoord = AttribTextCoord;

    gl_Position = projection * view * model * vec4(AttribPosition, 1.0);
}  