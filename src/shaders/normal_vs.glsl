#version 330 core

layout (location = 0) in vec3 AttribPosition;
layout (location = 2) in vec2 AttribTextCoord;

out vec2 TexCoord;

void main() {
    TexCoord = AttribTextCoord;

    gl_Position = vec4(AttribPosition, 1.0);
}