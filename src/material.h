#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.h"

class Material {
public:
    glm::vec3 diffuse = { 0.8f, 0.8f, 0.8f };
    glm::vec3 specular = { 0.1, 0.1f, 0.1f };
    float shininess = 2;

    Material(const glm::vec3 &diffuse, const glm::vec3 &specular, float shininess) :diffuse(diffuse), specular(specular), shininess(shininess) { }

    Material() { }
};

#endif