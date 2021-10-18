#include "Kernel.h"

float Kernel::h; // kernel radius
float Kernel::h2;
float Kernel::factorWPoly6;
float Kernel::factorGradWSpiky;

float Kernel::WPoly6(const glm::vec3 &r) {
    float rNorm2 = glm::dot(r, r);
    if (rNorm2 < h2) {
        float diff = h2 - rNorm2;
        return factorWPoly6 * diff * diff * diff;
    } else
        return 0.0f;
}

glm::vec3 Kernel::gradWSpiky(const glm::vec3 &r) {
    float rNorm = glm::length(r);
    if (rNorm > 1.0e-6f && rNorm < h) {
        return factorGradWSpiky * (h - rNorm) * (h - rNorm) / rNorm * r;
    } else
        return glm::vec3(0.0f);
}

void Kernel::setKernelRadius(float kernelRadius) {
    h = kernelRadius;
    h2 = h * h;
    factorWPoly6 = 315.0f / (64.0f * glm::pi<float>() * powf(h, 9.0f));
    factorGradWSpiky = -45.0f / (glm::pi<float>() * powf(h, 6.0f));
}