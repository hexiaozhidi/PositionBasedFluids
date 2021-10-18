#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform int numGridCells;
uniform vec3 gridCellColorA;
uniform vec3 gridCellColorB;

void main() {
    vec2 texCoord = fract(TexCoord * numGridCells * 0.5);
    if (texCoord.x <= 0.5 && texCoord.y > 0.5 || texCoord.x > 0.5 && texCoord.y <= 0.5)
        FragColor = vec4(gridCellColorA, 1.0);
    else
        FragColor = vec4(gridCellColorB, 1.0);
}