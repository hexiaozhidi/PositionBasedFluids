#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.h"

class PointLight {
public:
    glm::vec3 position = { 0.0f, 3.0f, 2.0f };

    glm::vec3 ambient = { 0.4f, 0.4f, 0.4f };
    glm::vec3 diffuse = { 0.95f, 0.95f, 0.95f };
    glm::vec3 specular = { 1.0f, 1.0f, 1.0f };

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    PointLight(const glm::vec3 &position, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular,
        float constant, float linear, float quadratic) :
        position(position), ambient(ambient), diffuse(diffuse), specular(specular), constant(constant), linear(linear), quadratic(quadratic) {
    }

    PointLight(const glm::vec3 &position, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular) :
        position(position), ambient(ambient), diffuse(diffuse), specular(specular) {
    }

    PointLight(const glm::vec3 &position) : position(position) { }

    PointLight() { }
};

#endif