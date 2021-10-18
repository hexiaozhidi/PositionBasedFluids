#ifndef STAGE_H
#define STAGE_H

#include <glm/glm.hpp>

#include <vector>

#include "mesh.h"

class Stage : public Mesh {
public:
    Stage() {
        vertices = {
            // back
            { 0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f }, // top-right
            { -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f }, // top-left
            { -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f }, // bottom-left
            { 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f }, // bottom-right

            // left
            { -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f }, // top-right
            { -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f }, // top-left
            { -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f }, // bottom-left
            { -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f }, // bottom-right

            // right
            { 0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f }, // top-right
            { 0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f }, // top-left
            { 0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f }, // bottom-left
            { 0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f }, // bottom-right

            // bottom
            { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f }, // top-right
            { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f }, // top-left
            { -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f }, // bottom-left
            { 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f }, // bottom-right

            // top
            { 0.5f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f }, // top-right
            { -0.5f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f }, // top-left
            { -0.5f, 0.5f, -0.5f, 0.0f, -1.0f, 0.0f }, // bottom-left
            { 0.5f, 0.5f, -0.5f, 0.0f, -1.0f, 0.0f } // bottom-right
        };

        indices = {
            0, 1, 2, 0, 2, 3, // back
            4, 5, 6, 4, 6, 7, // left
            8, 9, 10, 8, 10, 11, // right
            12, 13, 14, 12, 14, 15, // bottom
            16, 17, 18, 16, 18, 19 // top
        };
        initialize();
    }
};

#endif