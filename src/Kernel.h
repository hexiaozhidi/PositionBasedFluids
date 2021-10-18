#ifndef KERNEL_H
#define KERNEL_H

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

class Kernel {
private:
    static float h; // kernel radius
    static float h2;
    static float factorWPoly6;
    static float factorGradWSpiky;

public:
    static float WPoly6(const glm::vec3 &r);
    static glm::vec3 gradWSpiky(const glm::vec3 &r);
    static void setKernelRadius(float kernelRadius);
};

#endif