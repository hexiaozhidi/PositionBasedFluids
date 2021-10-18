#ifndef STATICOBJECT_H
#define STATICOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh/Mesh.h"
#include "shader.h"

class StaticObject {
public:
    Mesh &mesh;

    glm::vec3 position; // world-space position of the center of the object
    glm::vec3 scale;
    float rotateAngle;
    glm::vec3 rotateAxis;

    glm::mat4 model;
    glm::mat3 transInvModel;

    StaticObject(Mesh &mesh, const glm::vec3 &position, const glm::vec3 &scale, float rotateAngle, const glm::vec3 &rotateAxis) :
        mesh(mesh), position(position), scale(scale), rotateAngle(rotateAngle), rotateAxis(rotateAxis) {
        updateModelMatrix();
    }

    StaticObject(Mesh &mesh, const glm::vec3 &position, const glm::vec3 &scale) : StaticObject(mesh, position, scale, 0.0f, { 0.0f, 1.0f, 0.0f }) { }

    StaticObject(Mesh &mesh, const glm::vec3 &position) : StaticObject(mesh, position, { 1.0f, 1.0f, 1.0f }) { }

    StaticObject(Mesh &mesh) : StaticObject(mesh, { 0.0f, 1.0f, 0.0f }) { }

    void draw(Shader &shader) {
        mesh.draw(shader);
    }

    glm::mat4 getModelMatrix() {
        return model;
    }

    glm::mat3 getTransInvModelMatrix() {
        return transInvModel;
    }

    void updateModelMatrix() {
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotateAngle), rotateAxis);
        model = glm::scale(model, scale);
        transInvModel = glm::transpose(glm::inverse(glm::mat3(model)));
    }
};

#endif