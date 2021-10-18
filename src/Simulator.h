#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <omp.h>

#include <glm/glm.hpp>

#include <vector>

#include "Kernel.h"

enum class SceneType {
    DEFAULT,
    BILLOW,
    SPOUT
};

class Simulator {
public:
    using vector2d_int = std::vector<std::vector<int>>;

    SceneType sceneType = SceneType::DEFAULT;

    bool isPaused = true;

    int numIteration = 4;

    float restDensity = 6378.0f;
    float invRestDensity = 1.0f / restDensity;
    float invRestDensity2 = invRestDensity * invRestDensity;
    float massFactor = glm::pi<float>() / 6.0f;

    glm::vec3 gravity = { 0.0f, -9.80665f, 0.0f };
    float epsilonCFM = 600.0f; // CFM parameter
    float c = 0.000001f; // artificial viscosity
    float sCorr = -0.0016f; // artificial pressure strength
    float epsilonVC = 1.0e-6f; // vorticity confinement parameter

    float kernelRadius = 0.1f;

    float timeStep = 0.0f;
    float invTimeStep = 0.0f;

    float particleRadius = 0.0f;
    float particleDiameter = 0.0f;
    float neighbourDistance = 0.0f;
    float neighbourDistance2 = 0.0f;

    glm::ivec3 fluidSize;
    glm::vec3 fluidCornerPosition;
    glm::ivec3 containerSize;
    glm::vec3 containerCornerPosition;

    float mass = 0.0f;

    int numFluidParticles = 0;
    int numBoundaryParticles = 0;
    int numParticles = 0;

    std::vector<glm::vec3> positions; // positions of fluid particles and fixed boundary particles
    glm::vec3 positionMin;
    glm::vec3 positionMax;

    std::vector<glm::vec3> lastPositions; // positions of fluid particles in the last timestep
    std::vector<glm::vec3> velocities; // velocities of fluid particles
    std::vector<float> densities; // densities of fluid particles
    std::vector<float> lambdas; // lambda values of fluid particles
    std::vector<glm::vec3> deltaPositions; // correction of positions of fluid particles
    std::vector<glm::vec3> deltaVelocities; // correction of velocities of fluid particles (for applying vorticity confinement and XPSH viscosity)

    std::vector<float> psis; // psi values of boundary particles

    float gridCellSize = 0.0f;
    float invGridCellSize = 0.0f;
    glm::ivec3 gridCellIndexMin; // absolute min grid cell index (can be negative)
    glm::ivec3 gridCellIndexMax; // absolute max grid cell index
    glm::ivec3 gridSize;
    int gridSizeYZ = 0;
    int gridSizeXYZ = 0;
    vector2d_int fluidGrid;
    vector2d_int boundaryGrid;

    vector2d_int neighbourIndices; // indices of neighbours of particles

    Simulator(SceneType sceneType, float timeStep, float particleRadius, const glm::ivec3 &fluidSize, const glm::vec3 &fluidCornerPosition,
        const glm::ivec3 &containerSize, const glm::vec3 &containerCornerPosition) :
        sceneType(sceneType), timeStep(timeStep), particleRadius(particleRadius),
        fluidSize(fluidSize), fluidCornerPosition(fluidCornerPosition),
        containerSize(containerSize), containerCornerPosition(containerCornerPosition) {
        reset();
    }

    void simulate() {
        if (isPaused)
            return;

        // apply gravity
        applyGravity();

        // predict positions
        predictPositions();

        // find neighbours of fluid particles
        updateGrid(positions, 0, numFluidParticles, fluidGrid);
        findNeighbours(positions, neighbourIndices, 0, numFluidParticles, { &fluidGrid, &boundaryGrid });

        // solve density constraints
        for (int iter = 0; iter < numIteration; ++iter) {
            // calculate densities
            calculateDensities();

            // calculate lambdas
            calculateLambdas();

            // calculate corrections of positions
            calculateCorrectionsOfPositions();

            // correct positions
            correctPositions();
        }

        // predict velocities
        predictVelocities();

        // applying vorticity confinement
        applyVorticityConfinement();

        // applying XSPH viscosity
        applyXSPHViscosity();
    }

    void pause() { isPaused = !isPaused; }

    void reset() {
        isPaused = true;

        // set time step
        setTimeStep();

        // set radius and kernel
        setRadius();
        Kernel::setKernelRadius(kernelRadius);

        // set mass of a fluid particl
        setMass();

        // create fluid particles
        createFluidParticles(fluidSize, fluidCornerPosition, containerSize, containerCornerPosition);

        // create boundary particles
        createBoundaryParticles(containerSize, containerCornerPosition);

        // initialize grid for finding neighbours
        initializeGrid(containerSize, containerCornerPosition);

        // find boundary neighbours of boundary particles
        updateGrid(positions, numFluidParticles, numParticles, boundaryGrid);
        findNeighbours(positions, neighbourIndices, numFluidParticles, numParticles, { &boundaryGrid });

        // set boundary psi values
        setPsis();
    }

    void setTimeStep() {
        invTimeStep = 1.0f / timeStep;
    }

    void setRadius() {
        particleDiameter = 2.0f * particleRadius;
        neighbourDistance = 4.0f * particleRadius;
        neighbourDistance2 = neighbourDistance * neighbourDistance;
    }

    void setMass() {
        mass = restDensity * massFactor * particleDiameter * particleDiameter * particleDiameter;
    }

    void createFluidParticles(const glm::ivec3 &fluidSize, const glm::vec3 &fluidCornerPosition,
        const glm::ivec3 &containerSize, const glm::vec3 &containerCornerPosition) {
        positionMin = containerCornerPosition + particleDiameter;
        positionMax = positionMin + glm::vec3(containerSize) * particleDiameter;

        positions.clear();

        glm::vec3 fluidPosition = containerCornerPosition + particleDiameter + fluidCornerPosition + particleRadius;

        switch (sceneType) {
            case (SceneType::BILLOW):
                for (int i = 0; i < fluidSize.x; ++i)
                    for (int j = 0; j < fluidSize.y; ++j)
                        for (int k = 0; k < fluidSize.z; ++k)
                            positions.push_back(fluidPosition + glm::vec3(i, j, k) * particleDiameter);

                for (int i = 0; i < 2.0f * fluidSize.x; ++i)
                    for (int j = 0; j < 0.4f * fluidSize.y; ++j)
                        for (int k = fluidSize.z; k < 1.5f * fluidSize.z; ++k)
                            positions.push_back(fluidPosition + glm::vec3(i, j, k) * particleDiameter);

                break;
            case (SceneType::SPOUT):
                for (int i = 0; i < 3.0f * fluidSize.x; ++i)
                    for (int j = 0; j < 0.2f * fluidSize.y; ++j)
                        for (int k = 0; k < 2.0f * fluidSize.z; ++k)
                            positions.push_back(fluidPosition + glm::vec3(i, j, k) * particleDiameter);

                for (int i = 1.3f * fluidSize.x; i < 1.7f * fluidSize.x; ++i)
                    for (int j = 0.4f * fluidSize.y; j < 2.4f * fluidSize.y; ++j)
                        for (int k = 0.8f * fluidSize.z; k < 1.2f * fluidSize.z; ++k) {
                            glm::vec3 position = fluidPosition + glm::vec3(i, j, k) * particleDiameter;
                            position.x = containerCornerPosition.x + containerSize.x * particleDiameter + particleRadius - i * particleDiameter;
                            positions.push_back(position);
                        }

                break;
            default:
                for (int i = 0; i < fluidSize.x; ++i)
                    for (int j = 0; j < fluidSize.y; ++j)
                        for (int k = 0; k < fluidSize.z; ++k)
                            positions.push_back(fluidPosition + glm::vec3(i, j, k) * particleDiameter);
        }

        numFluidParticles = positions.size();

        lastPositions.clear();
        lastPositions.assign(positions.begin(), positions.begin() + numFluidParticles);

        velocities.clear();
        velocities.resize(numFluidParticles);

        densities.clear();
        densities.resize(numFluidParticles);

        lambdas.clear();
        lambdas.resize(numFluidParticles);

        deltaPositions.clear();
        deltaPositions.resize(numFluidParticles);

        deltaVelocities.clear();
        deltaVelocities.resize(numFluidParticles);
    }

    void createBoundaryParticles(const glm::ivec3 &containerSize, const glm::vec3 &containerCornerPosition) {
        glm::vec3 boundaryPosition = containerCornerPosition + particleRadius;
        for (int i = 0; i < containerSize.x + 2; ++i)
            for (int j = 0; j < containerSize.y + 2; ++j) {
                positions.push_back(boundaryPosition + glm::vec3(i, j, 0) * particleDiameter); // back
                positions.push_back(boundaryPosition + glm::vec3(i, j, containerSize.z + 1) * particleDiameter); // front
            }

        for (int j = 0; j < containerSize.y + 2; ++j)
            for (int k = 1; k <= containerSize.z; ++k) {
                positions.push_back(boundaryPosition + glm::vec3(0, j, k) * particleDiameter); // left
                positions.push_back(boundaryPosition + glm::vec3(containerSize.x + 1, j, k) * particleDiameter); // right
            }

        for (int i = 1; i <= containerSize.x; ++i)
            for (int k = 1; k <= containerSize.z; ++k) {
                positions.push_back(boundaryPosition + glm::vec3(i, 0, k) * particleDiameter); // bottom
                positions.push_back(boundaryPosition + glm::vec3(i, containerSize.y + 1, k) * particleDiameter); // top
            }

        numParticles = positions.size();
        numBoundaryParticles = numParticles - numFluidParticles;

        psis.clear();
        psis.resize(numBoundaryParticles);
    }

    void initializeGrid(const glm::ivec3 &containerSize, const glm::vec3 &containerCornerPosition) {
        gridCellSize = neighbourDistance;
        invGridCellSize = 1.0f / gridCellSize;
        gridCellIndexMin = glm::floor(containerCornerPosition * invGridCellSize) - 1.0f;
        gridCellIndexMax = glm::floor((containerCornerPosition + glm::vec3(containerSize + 2) * particleDiameter) * invGridCellSize) + 1.0f;
        gridSize = gridCellIndexMax - gridCellIndexMin + 1;

        gridSizeYZ = gridSize.y * gridSize.z;
        gridSizeXYZ = gridSize.x * gridSizeYZ;

        fluidGrid.clear();
        fluidGrid.resize(gridSizeXYZ);

        boundaryGrid.clear();
        boundaryGrid.resize(gridSizeXYZ);

        neighbourIndices.clear();
        neighbourIndices.resize(numParticles);
    }

    void updateGrid(const std::vector<glm::vec3> &positions, int rangeBegin, int rangeEnd, vector2d_int &grid) {

        for (auto &cell : grid)
            cell.clear();

        for (int i = rangeBegin; i < rangeEnd; ++i) {
            glm::ivec3 gridCellIndex = glm::floor(positions[i] * invGridCellSize); // absolute index
            if (glm::all(glm::greaterThanEqual(gridCellIndex, gridCellIndexMin)) &&
                glm::all(glm::lessThanEqual(gridCellIndex, gridCellIndexMax))) {
                gridCellIndex -= gridCellIndexMin; // relative index
                grid[gridCellIndex.x * gridSizeYZ + gridCellIndex.y * gridSize.z + gridCellIndex.z].push_back(i);
            }
        }
    }

    void findNeighbours(const std::vector<glm::vec3> &positions, vector2d_int &neighbourIndices,
        int sourceRangeBegin, int sourceRangeEnd, const std::vector<vector2d_int *> grids) {
        #pragma omp parallel default(shared)
            {
                #pragma omp for schedule(static)
                for (int i = sourceRangeBegin; i < sourceRangeEnd; ++i) {
                    neighbourIndices[i].clear();

                    const glm::vec3 &pi = positions[i];
                    const glm::ivec3 sourceCellIndex = glm::floor(pi * invGridCellSize); // absolute index

                    for (int dx = -1; dx < 2; ++dx)
                        for (int dy = -1; dy < 2; ++dy)
                            for (int dz = -1; dz < 2; ++dz) {
                                glm::ivec3 targetCellIndex = sourceCellIndex + glm::ivec3(dx, dy, dz);
                                if (glm::all(glm::greaterThanEqual(targetCellIndex, gridCellIndexMin)) &&
                                    glm::all(glm::lessThanEqual(targetCellIndex, gridCellIndexMax))) {
                                    targetCellIndex -= gridCellIndexMin;
                                    int targetCellNumber = targetCellIndex.x * gridSizeYZ + targetCellIndex.y * gridSize.z + targetCellIndex.z;

                                    for (auto grid : grids)
                                        for (int j : (*grid)[targetCellNumber]) {
                                            glm::vec3 diff = pi - positions[j];
                                            if (glm::dot(diff, diff) < neighbourDistance2)
                                                neighbourIndices[i].push_back(j);
                                        }
                                }
                            }
                }
            }
    }

    void setPsis() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = numFluidParticles; i < numParticles; ++i) {
                const glm::vec3 &pi = positions[i];
                float &psi = psis[i - numFluidParticles];
                for (int j : neighbourIndices[i])
                    psi += Kernel::WPoly6(pi - positions[j]);
                psi = restDensity / psi;
            }
        }
    }

    void applyGravity() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                velocities[i] += timeStep * gravity;
            }
        }
    }

    void predictPositions() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                glm::vec3 &pi = positions[i];
                glm::vec3 &vi = velocities[i];
                pi += timeStep * vi;

                if (pi.x < positionMin.x) {
                    pi.x = positionMin.x + particleDiameter;
                    vi.x *= -0.5f;
                } else if (pi.x > positionMax.x) {
                    pi.x = positionMax.x - particleDiameter;
                    vi.x *= -0.5f;
                }

                if (pi.y < positionMin.y) {
                    pi.y = positionMin.y + particleDiameter;
                    vi.y *= -0.5f;
                } else if (pi.y > positionMax.y) {
                    pi.y = positionMax.y - particleDiameter;
                    vi.y *= -0.5f;
                }

                if (pi.z < positionMin.z) {
                    pi.z = positionMin.z + particleDiameter;
                    vi.z *= -0.5f;
                } else if (pi.z > positionMax.z) {
                    pi.z = positionMax.z - particleDiameter;
                    vi.z *= -0.5f;
                }
            }
        }
    }

    void calculateDensities() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                const glm::vec3 &pi = positions[i];
                float &density = densities[i];

                density = 0.0f;

                for (int j : neighbourIndices[i]) {
                    float m = j < numFluidParticles ? mass : psis[j - numFluidParticles];
                    density += m * Kernel::WPoly6(pi - positions[j]);
                }
            }
        }
    }

    void calculateLambdas() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                lambdas[i] = 0.0f;
                //if (densities[i] < restDensity) // negative Ci
                //    continue;

                const glm::vec3 &pi = positions[i];
                float &lambda = lambdas[i];

                glm::vec3 gradConstraint(0.0f); // gradient of Ci with respect to pi (without multiplying invRestDensity)

                for (int j : neighbourIndices[i]) {
                    float m = j < numFluidParticles ? mass : psis[j - numFluidParticles];
                    glm::vec3 grad = m * Kernel::gradWSpiky(pi - positions[j]);
                    gradConstraint += grad;
                    lambda += glm::dot(grad, grad);
                }

                lambda = (1 - densities[i] * invRestDensity) /
                    (invRestDensity2 * (lambda + glm::dot(gradConstraint, gradConstraint)) + epsilonCFM);
            }
        }
    }

    void calculateCorrectionsOfPositions() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                const glm::vec3 &pi = positions[i];
                const float &lambdai = lambdas[i];
                glm::vec3 &deltaPosition = deltaPositions[i];

                deltaPosition = glm::vec3(0.0f);

                for (int j : neighbourIndices[i])
                    if (j < numFluidParticles)
                        deltaPosition += (lambdai + lambdas[j] + sCorr) * mass * Kernel::gradWSpiky(pi - positions[j]);
                    else
                        deltaPosition += (lambdai + sCorr) * psis[j - numFluidParticles] * Kernel::gradWSpiky(pi - positions[j]);

                deltaPosition *= invRestDensity;
            }
        }
    }

    void correctPositions() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i)
                positions[i] += deltaPositions[i];
        }
    }

    void predictVelocities() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                velocities[i] = invTimeStep * (positions[i] - lastPositions[i]);
                lastPositions[i] = positions[i];
            }
        }
    }

    void applyVorticityConfinement() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                const glm::vec3 &pi = positions[i];
                const glm::vec3 &vi = velocities[i];

                float numFluidNeighbours = 0.0f;
                glm::vec3 eta(0.0f);
                glm::vec3 omega(0.0f);

                for (int j : neighbourIndices[i])
                    if (j < numFluidParticles) {
                        eta += positions[j];
                        omega += glm::cross(velocities[j] - vi, Kernel::gradWSpiky(positions[j] - pi));
                        ++numFluidNeighbours;
                    }

                eta = 0.5f * (eta - numFluidNeighbours * pi);
                float etaNorm = glm::length(eta);
                deltaVelocities[i] = etaNorm > 1.0e-6f ? epsilonVC * glm::cross(eta / etaNorm, omega) : glm::vec3(0.0f);
            }
        }

        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i)
                velocities[i] += deltaVelocities[i];
        }
    }

    void applyXSPHViscosity() {
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i) {
                const glm::vec3 &pi = positions[i];
                const glm::vec3 &vi = velocities[i];
                glm::vec3 &deltaVelocity = deltaVelocities[i];

                deltaVelocity = glm::vec3(0.0f);

                for (int j : neighbourIndices[i])
                    if (j < numFluidParticles)
                        //deltaVelocity += (velocities[j] - vi) * Kernel::WPoly6(pi - positions[j]);
                        deltaVelocity += (velocities[j] - vi) * Kernel::WPoly6(pi - positions[j]) / densities[j];

                //deltaVelocity *= c;
                deltaVelocity *= c * mass;
            }
        }

        // correct velocities (by applying XSPH viscosity)
        #pragma omp parallel default(shared)
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < numFluidParticles; ++i)
                velocities[i] += deltaVelocities[i];
        }
    }
};

#endif