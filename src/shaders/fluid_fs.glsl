#version 330 core

in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

uniform samplerCube skybox;
uniform sampler2D mapSmoothedDepth;
uniform sampler2D mapThickness;
uniform sampler2D mapNormal;

uniform float floorSize;
uniform int numGridCells;
uniform vec3 gridCellColorA;
uniform vec3 gridCellColorB;

uniform vec3 cameraPosition;
uniform mat4 viewInv;
uniform float tanHalfFovy;
uniform float aspect;

uniform float refractionIndexRatio; // eta_i / eta_r
uniform float R0;
uniform vec3 tintColor;
uniform float schlickPower;

vec3 traceColor(vec3 fragPosition, vec3 direction);

void main() {
    float fragPositionZViewSpace = -texture(mapSmoothedDepth, TexCoord).r;
    float fragPositionXViewSpace = -aspect * tanHalfFovy * fragPositionZViewSpace * (2.0 * TexCoord.x - 1.0);
    float fragPositionYViewSpace = -tanHalfFovy * fragPositionZViewSpace * (2.0 * TexCoord.y - 1.0);
    vec3 fragPositionViewSpace = vec3(fragPositionXViewSpace, fragPositionYViewSpace, fragPositionZViewSpace);
    vec3 fragPosition = vec3(viewInv * vec4(fragPositionViewSpace, 1.0));
    
    vec3 incidentDirection = normalize(fragPosition - cameraPosition);
    vec3 normal = normalize(mat3(viewInv) * texture(mapNormal, TexCoord).rgb);

    vec3 reflectionDirection = reflect(incidentDirection, normal);
    vec3 reflectionColor = traceColor(fragPosition, reflectionDirection);

    vec3 refractionDirection = refract(incidentDirection, normal, refractionIndexRatio);
    vec3 refractionColor = traceColor(fragPosition, refractionDirection);
    float thickness = texture(mapThickness, TexCoord).r;
    float attenuation = max(exp(-100.0 * thickness), 0.2);
    refractionColor = mix(tintColor, refractionColor, attenuation);

    float R = 1.0 - dot(-incidentDirection, normal);
    R = R0 + (1.0 - R0) * pow(R, schlickPower);

    FragColor = vec4(mix(refractionColor, reflectionColor, R), 1.0);
}

vec3 traceColor(vec3 fragPosition, vec3 direction) {    
    if (direction.y >= 0.0)
        return texture(skybox, direction).rgb;
    
    float t = -fragPosition.y / direction.y;
    float intersectionX = fragPosition.x + t * direction.x;
    float intersectionZ = fragPosition.z + t * direction.z;
    float edge = 0.5 * floorSize;

    if (min(intersectionX, intersectionZ) < -edge || max(intersectionX, intersectionZ) > edge)
        return texture(skybox, direction).rgb;

    vec2 texCoord = vec2(intersectionX, intersectionZ) / floorSize + 0.5;
    texCoord = fract(texCoord * numGridCells * 0.5);
    return texCoord.x <= 0.5 && texCoord.y > 0.5 || texCoord.x > 0.5 && texCoord.y <= 0.5 ? gridCellColorA : gridCellColorB;
}