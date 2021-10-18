#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <vector>

#include "Mesh.h"

class Sphere : public Mesh {
public:
    Sphere(float radius, int numLatitudeZones, int numLongitudeZones) {
        float dphi = glm::pi<float>() / numLatitudeZones;
        float dtheta = glm::pi<float>() * 2 / numLongitudeZones;
        int countVertex = 0;

        for (int latitude = 0; latitude <= numLatitudeZones; ++latitude) {
            float phi = latitude * dphi;
            float y = cosf(phi);
            float r = sinf(phi);

            for (int longitude = 0; longitude <= numLongitudeZones; ++longitude) {
                float theta = longitude * dtheta;
                float x = r * cosf(theta);
                float z = -r * sinf(theta);

                vertices.emplace_back(x * radius, y * radius, z * radius, x, y, z);
                if (latitude != numLatitudeZones && longitude != numLongitudeZones) {
                    // triangle 1
                    indices.push_back(countVertex + 1); // top-right
                    indices.push_back(countVertex); // top-left
                    indices.push_back(countVertex + numLongitudeZones + 1); // bottom-left

                    // triangle 2
                    indices.push_back(countVertex + 1); // top-right
                    indices.push_back(countVertex + numLongitudeZones + 1); // bottom-left
                    indices.push_back(countVertex + numLongitudeZones + 2); // bottom-right
                }

                ++countVertex;
            }
        }

        initialize();
    }

    Sphere(float radius) : Sphere(radius, 18, 36) { }

    Sphere() : Sphere(1.0, 18, 36) { }
};

#endif