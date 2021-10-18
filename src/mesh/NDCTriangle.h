#ifndef NDCTRIANGLE_H
#define NDCTRIANGLE_H

#include <glm/glm.hpp>

#include <vector>

#include "Mesh.h"

class NDCTriangle : public Mesh {
public:
    NDCTriangle() {
        vertices = {
            { -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, // bottom-left
            { 3.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f }, // bottom-right
            { -1.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f }, // top-left
        };

        indices = { 0, 1, 2 };
        initialize();
    }
};

#endif