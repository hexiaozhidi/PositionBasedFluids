#version 330 core

in vec3 CenterPositionViewSpace;
in vec3 FragPositionViewSpace;

layout (location = 3) out float Thickness;

void main() {
    vec3 dirFromCameraToFrag = normalize(FragPositionViewSpace);

    vec3 dirFromFragToCenter = CenterPositionViewSpace - FragPositionViewSpace;
    float radius = length(dirFromFragToCenter);
    dirFromFragToCenter /= radius; // noramlization

    Thickness = dot(dirFromCameraToFrag, dirFromFragToCenter) * radius * 2.0;
}