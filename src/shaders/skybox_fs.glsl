#version 330 core

in vec3 FragPosition;

out vec4 FragColor;

uniform samplerCube skybox;

void main() {    
    FragColor = vec4(texture(skybox, FragPosition).rgb, 1.0);
}