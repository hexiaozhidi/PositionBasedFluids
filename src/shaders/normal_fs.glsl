#version 330 core

in vec2 TexCoord;

layout (location = 4) out vec3 Normal;

uniform sampler2D mapDepth;
uniform float C;

void main() {
    vec2 texelSize = 1.0 / textureSize(mapDepth, 0);
	float depth = texture(mapDepth, TexCoord).r;

    float forwardDifferenceX = texture(mapDepth, vec2(TexCoord.x + texelSize.x, TexCoord.y)).r - depth;
    float backwardDifferenceX = depth - texture(mapDepth, vec2(TexCoord.x - texelSize.x, TexCoord.y)).r;
    float differenceX = abs(forwardDifferenceX) < abs(backwardDifferenceX) ? forwardDifferenceX : backwardDifferenceX;

    float forwardDifferenceY = texture(mapDepth, vec2(TexCoord.x, TexCoord.y + texelSize.y)).r - depth;
    float backwardDifferenceY = depth - texture(mapDepth, vec2(TexCoord.x, TexCoord.y - texelSize.y)).r;
    float differenceY = abs(forwardDifferenceY) < abs(backwardDifferenceY) ? forwardDifferenceY : backwardDifferenceY;

    Normal = normalize(vec3(differenceX, differenceY, -C * depth));
}