#version 330 core

in vec2 TexCoord;

layout (location = 1) out float SmoothedDepthA;
layout (location = 2) out float SmoothedDepthB;

uniform sampler2D mapDepth;
uniform bool fromAtoB;
uniform int windowRadius;
uniform float rangeFactor;
uniform float spatialFactor;

void main() {
	vec2 texelSize = 1.0 / textureSize(mapDepth, 0);
	float depth = texture(mapDepth, TexCoord).r;

	float sum = 0.0;
	float sumWeight = 0.0;

	for (int i = -windowRadius; i <= windowRadius; ++i)
		for (int j = -windowRadius; j <= windowRadius; ++j) {
			vec2 deltaTexCoord = vec2(i, j) * texelSize;
			vec2 sampleTexCoord = TexCoord + deltaTexCoord;
			float sampleDepth = texture(mapDepth, sampleTexCoord).r;
			
			float rangeTerm = (sampleDepth - depth) * rangeFactor;
			rangeTerm *= rangeTerm;
			float spatialTerm = dot(deltaTexCoord, deltaTexCoord) * spatialFactor * spatialFactor;
			float weight = exp(-spatialTerm - rangeTerm);

			sum += weight * sampleDepth;
			sumWeight += weight;
		}

	float smoothedDepth = sumWeight > 1.0e-6 ? sum / sumWeight : 0.0;

    if (fromAtoB)
        SmoothedDepthA = smoothedDepth;
    else
        SmoothedDepthB = smoothedDepth;
}