#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

#include "../shader.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    Vertex(const glm::vec3 &position) : position(position) { }

    Vertex(const glm::vec3 &position, const glm::vec3 &normal) : position(position), normal(normal) { }

    Vertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texCoord) : position(position), normal(normal), texCoord(texCoord) { }

    Vertex(float px, float py, float pz) : position({ px, py, pz }) { }
    
    Vertex(float px, float py, float pz, float nx, float ny, float nz) : position({ px, py, pz }), normal({ nx, ny, nz }) { }

    Vertex(float px, float py, float pz, float nx, float ny, float nz, float tcx, float tcy) : position({ px, py, pz }), normal({ nx, ny, nz }), texCoord({ tcx, tcy }) { }
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;

    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices) : vertices(vertices), indices(indices) {
        initialize();
    }

    Mesh() {
        initialize();
    }

    void draw(Shader &shader) {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void initialize() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

        glBindVertexArray(0);
    }

    void clear() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

#endif