#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>

#include <vector>

#include "Mesh.h"

class Plane : public Mesh {
public:
    Plane() {
        vertices = {
            { 0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f }, // top-right
            { -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f }, // top-left
            { -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f }, // bottom-left
            { 0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f }, // bottom-right
        };

        indices = { 0, 1, 2, 0, 2, 3 };
        initialize();
    }
};

#endif