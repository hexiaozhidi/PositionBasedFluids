#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh/Mesh.h"
#include "shader.h"

class Object {
public:
    Mesh &mesh;

    Object(Mesh &mesh) : mesh(mesh) { }

    void draw(Shader &shader) {
        mesh.draw(shader);
    }

    glm::mat4 getModelMatrix(const glm::vec3 &translation) {
        return glm::translate(glm::mat4(1.0f), translation);
    }

    glm::mat4 getModelMatrix(const glm::vec3 &translation, const glm::vec3 &scale) {
        return glm::scale(getModelMatrix(translation), scale);
    }

    glm::mat4 getModelMatrix(const glm::vec3 &translation, const glm::vec3 &scale, float rotateAngle, const glm::vec3 &rotateAxis) {
        return glm::scale(glm::rotate(getModelMatrix(translation), glm::radians(rotateAngle), rotateAxis), scale);
    }
};

#endif