#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D mapScene;
uniform sampler2D mapDepth;
uniform sampler2D mapSmoothedDepth;
uniform sampler2D mapThickness;
uniform sampler2D mapNormal;
uniform int displayMode;

vec4 displayDepth(vec2 texCoord);
vec4 displaySmoothedDepth(vec2 texCoord);
vec4 displayThickness(vec2 texCoord);
vec4 displayNormal(vec2 texCoord);
vec4 displayScene(vec2 texCoord);

void main() {
    switch (displayMode) {
        case 0:
            vec2 texCoord = fract(3.0 * TexCoord);
            if (TexCoord.y > 0.666667) {
                if (TexCoord.x <= 0.333333)
                    FragColor = displayDepth(texCoord);
                else if (TexCoord.x <= 0.666667)
                    FragColor = displaySmoothedDepth(texCoord);
                else
                    FragColor = displayThickness(texCoord);
            } else if (TexCoord.x > 0.666667) {
                if (TexCoord.y > 0.333333)
                    FragColor = displayNormal(texCoord);
                else
                    FragColor = displayScene(texCoord);
            } else {
                texCoord = fract(1.5 * TexCoord);
                FragColor = displayScene(texCoord);
            }
            break;
        case 1:
            FragColor = displayDepth(TexCoord);
            break;
        case 2:
            FragColor = displaySmoothedDepth(TexCoord);
            break;
        case 3:
            FragColor = displayThickness(TexCoord);
            break;
        case 4:
            FragColor = displayNormal(TexCoord);
            break;
        case 5:
            FragColor = displayScene(TexCoord);
            break;
     }
}

vec4 displayDepth(vec2 texCoord) {
    return vec4((texture(mapDepth, texCoord).rrr - 9.0) * 0.8, 1.0);
}

vec4 displaySmoothedDepth(vec2 texCoord) {
    return vec4((texture(mapSmoothedDepth, texCoord).rrr - 9.0) * 0.8, 1.0);
}

vec4 displayThickness(vec2 texCoord) {
    return vec4(texture(mapThickness, texCoord).rrr * 0.3 , 1.0);
}

vec4 displayNormal(vec2 texCoord) {
    return vec4(texture(mapNormal, texCoord).rgb, 1.0);
}

vec4 displayScene(vec2 texCoord) {
    return vec4(texture(mapScene, texCoord).rgb, 1.0);
}