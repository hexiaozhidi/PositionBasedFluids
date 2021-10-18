#ifndef NDCSQUARE_H
#define NDCSQUARE_H

#include <glm/glm.hpp>

#include <vector>

#include "Mesh.h"

class NDCSquare : public Mesh {
public:
    NDCSquare() {
        vertices = {
            { 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f }, // top-right
            { -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }, // top-left
            { -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, // bottom-left
            { 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }, // bottom-right
        };

        indices = { 0, 1, 2, 0, 2, 3 };
        initialize();
    }
};

#endif