#version 330 core

in float FragPositionZViewSpace;

layout (location = 0) out float Depth;

void main() {
    Depth = -FragPositionZViewSpace;
}