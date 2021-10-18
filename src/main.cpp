#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "shader.h"
#include "camera.h"

#include "mesh/Cube.h"
#include "mesh/NDCTriangle.h"
#include "mesh/Plane.h"
#include "mesh/Sphere.h"

#include "StaticObject.h"
#include "Object.h"

#include "Simulator.h"

enum class DisplayMode {
    DEFAULT,
    DEPTH,
    SMOOTHED_DEPTH,
    THICKNESS,
    NORMAL,
    SCENE
};

void processKeyboardInput(GLFWwindow *window);
void processMouseMove(GLFWwindow *window, double deltaX, double deltaY);
void processMouseScroll(GLFWwindow *window, double deltaX, double deltaY);
unsigned int loadCubemap(const std::string &path, const std::string &type, bool gammaCorrection);
unsigned int loadTexture(const std::string &path, bool gammaCorrection);

// timer
float startTime = 0.0f;
float lastTime = 0.0f;
float deltaTime = 0.0f;
float sumDeltaTime = 0.0f;
int frameCount = 0;

// window
const unsigned int screenWidth = 1280;
const unsigned int screenHeight = 720;
DisplayMode displayMode = DisplayMode::DEFAULT;
bool printFPS = false;
bool fixFPS = false;

// key
bool pauseKeyPressed = false;
bool resetKeyPressed = false;
bool fixCameraKeyPressed = false;
bool displayDefaultKeyPressed = false;
bool displayDepth0KeyPressed = false;
bool displayDepth1KeyPressed = false;
bool displayThicknessKeyPressed = false;
bool displayNormalKeyPressed = false;
bool displaySceneKeyPressed = false;

// mouse
float lastMouseX = 0.5f * screenWidth;
float lastMouseY = 0.5f * screenHeight;
bool firstMouse = true;

// camera
Camera camera(glm::vec3(4.75f, 5.52f, 7.44f), glm::radians(-121.6f), glm::radians(-27.8f), glm::radians(30.0f), screenWidth / screenHeight);
bool rotateCamera = true;

// floor
float floorSize = 10.0f;
int numGridCells = 20;
glm::vec3 gridCellColorA(0.6f);
glm::vec3 gridCellColorB(1.0f);

// depth smoothing
int numSmoothing = 3;
int windowRadius = 10;
float rangeFactor = 4.0f; // smaller rangeFactor (larger sigmaRange) causes more blurred edges
float spatialFactor = 0.00001f; // smaller spatialFactor (larger sigmaSpatial) causes smoother surfaces

// scene shading
float airRefractionIndex = 1.0f;
float fluidRefractionIndex = 1.33f;
float R0 = powf((1.0f - fluidRefractionIndex) / (1.0f + fluidRefractionIndex), 2);
glm::vec3 tintColor(38.0f / 255.0f, 123.0f / 255.0f, 205.0f / 255.0f);
float schlickPower = 1.5f; // larger schlickPower causes more refraction and less reflection

// simulator
SceneType sceneType = SceneType::DEFAULT;

float timeStep = 0.0083f;

float particleDisplaySizeFactor = 2.0f;
float particleRadius = 0.03345f;
float particleDiameter = 2.0f * particleRadius;

glm::ivec3 fluidSize(20, 40, 20);
glm::vec3 fluidCornerPosition = glm::vec3(0.0f, 0.0f, 0.0f) * particleDiameter;

glm::ivec3 containerSize = sceneType == SceneType::BILLOW ? glm::vec3(3 * fluidSize.x, 3 * fluidSize.y, 1.5f * fluidSize.z) :
    sceneType == SceneType::SPOUT ? glm::vec3(3 * fluidSize.x, 3 * fluidSize.y, 2 * fluidSize.z) :
    glm::vec3(3 * fluidSize.x, 3 * fluidSize.y, fluidSize.z);

glm::vec3 containerCornerPosition = glm::vec3(-0.5f, 0.0f, -0.5f) * glm::vec3(containerSize) * particleDiameter;

glm::vec3 fluidPositionMin = containerCornerPosition + particleDiameter;
glm::vec3 fluidPositionMax = containerCornerPosition + particleDiameter + glm::vec3(containerSize) * particleDiameter;

Simulator simulator(sceneType, timeStep, particleRadius, fluidSize, fluidCornerPosition, containerSize, containerCornerPosition);

int main() {
    // set GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // create GLFW window
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "Position Based Fluids", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, processMouseMove);
    glfwSetScrollCallback(window, processMouseScroll);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // capture the mouse

    // set GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: Failed to initialize GLAD." << std::endl;
        return -1;
    }

    // set OpenGL parameters
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE);

    // create shaders
    Shader shaderDepth("src/shaders/depth_vs.glsl", "src/shaders/depth_fs.glsl");
    Shader shaderSmoothedDepth("src/shaders/smoothedDepth_vs.glsl", "src/shaders/smoothedDepth_fs.glsl");
    Shader shaderThickness("src/shaders/thickness_vs.glsl", "src/shaders/thickness_fs.glsl");
    Shader shaderNormal("src/shaders/normal_vs.glsl", "src/shaders/normal_fs.glsl");

    Shader shaderFluid("src/shaders/fluid_vs.glsl", "src/shaders/fluid_fs.glsl");
    Shader shaderFloor("src/shaders/floor_vs.glsl", "src/shaders/floor_fs.glsl");
    Shader shaderSkybox("src/shaders/skybox_vs.glsl", "src/shaders/skybox_fs.glsl");

    Shader shaderScreen("src/shaders/screen_vs.glsl", "src/shaders/screen_fs.glsl");

    // set texture units for shaders
    shaderSmoothedDepth.use();
    shaderSmoothedDepth.setInt("mapDepth", 0);
    shaderSmoothedDepth.setInt("windowRadius", windowRadius);
    shaderSmoothedDepth.setFloat("rangeFactor", rangeFactor);
    shaderSmoothedDepth.setFloat("spatialFactor", spatialFactor);

    shaderNormal.use();
    shaderNormal.setInt("mapDepth", 0);

    shaderFluid.use();
    shaderFluid.setInt("skybox", 0);
    shaderFluid.setInt("mapSmoothedDepth", 1);
    shaderFluid.setInt("mapThickness", 2);
    shaderFluid.setInt("mapNormal", 3);
    shaderFluid.setFloat("floorSize", floorSize);
    shaderFluid.setInt("numGridCells", numGridCells);
    shaderFluid.setVec3("gridCellColorA", gridCellColorA);
    shaderFluid.setVec3("gridCellColorB", gridCellColorB);
    shaderFluid.setFloat("refractionIndexRatio", airRefractionIndex / fluidRefractionIndex);
    shaderFluid.setFloat("R0", R0);
    shaderFluid.setVec3("tintColor", tintColor);
    shaderFluid.setFloat("schlickPower", schlickPower);

    shaderFloor.use();
    shaderFloor.setInt("numGridCells", numGridCells);
    shaderFloor.setVec3("gridCellColorA", gridCellColorA);
    shaderFloor.setVec3("gridCellColorB", gridCellColorB);

    shaderSkybox.use();
    shaderSkybox.setInt("skybox", 0);

    shaderScreen.use();
    shaderScreen.setInt("mapScene", 0);
    shaderScreen.setInt("mapDepth", 1);
    shaderScreen.setInt("mapSmoothedDepth", 2);
    shaderScreen.setInt("mapThickness", 3);
    shaderScreen.setInt("mapNormal", 4);

    // create a skybox
    Cube cube;
    Object skybox(cube);
    unsigned int cubemapSkybox = loadCubemap("resources/textures/skybox", "jpg", false);

    // create a floor
    Plane plane;
    Object floor(plane);

    // create particles
    Sphere sphere(particleDisplaySizeFactor * particleRadius, 9, 18);
    Object particle(sphere);

    // create a screen triangle
    NDCTriangle ndcTriangle;
    Object screenTriangle(ndcTriangle); // the screen zone is in the bottom-left of the triangle

    // set g-buffer
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gTextureDepth, gTextureSmoothedDepthA, gTextureSmoothedDepthB, gTextureThickness, gTextureNormal;

    glGenTextures(1, &gTextureDepth);
    glBindTexture(GL_TEXTURE_2D, gTextureDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gTextureDepth, 0);

    glGenTextures(1, &gTextureSmoothedDepthA);
    glBindTexture(GL_TEXTURE_2D, gTextureSmoothedDepthA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gTextureSmoothedDepthA, 0);

    glGenTextures(1, &gTextureSmoothedDepthB);
    glBindTexture(GL_TEXTURE_2D, gTextureSmoothedDepthB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gTextureSmoothedDepthB, 0);

    glGenTextures(1, &gTextureThickness);
    glBindTexture(GL_TEXTURE_2D, gTextureThickness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gTextureThickness, 0);

    glGenTextures(1, &gTextureNormal);
    glBindTexture(GL_TEXTURE_2D, gTextureNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gTextureNormal, 0);

    std::vector<unsigned int> gBufferAttachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(gBufferAttachments.size(), &gBufferAttachments[0]);

    unsigned int gRBO;
    glGenRenderbuffers(1, &gRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: Incomplete g-buffer." << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // set scene framebuffer
    unsigned int sceneBuffer;
    glGenFramebuffers(1, &sceneBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer);
    unsigned int sceneTexture;

    glGenTextures(1, &sceneTexture);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture, 0);

    std::vector<unsigned int> sceneBufferAttachments = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(sceneBufferAttachments.size(), &sceneBufferAttachments[0]);

    unsigned int sceneRBO;
    glGenRenderbuffers(1, &sceneRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: Incomplete scene framebuffer." << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create VBO of positions of particles
    unsigned int positionsVBO;
    glGenBuffers(1, &positionsVBO);
    glBindVertexArray(particle.mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
    glBufferData(GL_ARRAY_BUFFER, simulator.numFluidParticles * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);

    // record default camera parameters
    glm::vec3 cameraDefaultPosition = camera.position;
    float cameraMoveRadius = glm::length(cameraDefaultPosition - glm::vec3(0.0, cameraDefaultPosition.y, 0.0));
    float cameraDefaultTheta = atanf(cameraDefaultPosition.x / cameraDefaultPosition.z);
    float cameraDefaultYaw = camera.yaw;

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // process the input
        processKeyboardInput(window);

        // update the timer
        float time = glfwGetTime();
        deltaTime = time - lastTime;

        // rotate camera automatically
        if (rotateCamera) {
            float deltaTimeAfterStart = time - startTime;
            float deltaThetaAfterStart = deltaTimeAfterStart * 0.01;
            float theta = cameraDefaultTheta + deltaThetaAfterStart;
            camera.position = glm::vec3(cameraMoveRadius * sinf(theta), cameraDefaultPosition.y, cameraMoveRadius * cosf(theta));
            camera.yaw = cameraDefaultYaw - deltaThetaAfterStart;
            camera.updateAxes();
            camera.updateViewMatrix();
        }

        // fix FPS to 10
        if (fixFPS && deltaTime < 0.1)
            continue;

        lastTime = time;

        // show FPS
        if (printFPS) {
            sumDeltaTime += deltaTime;
            ++frameCount;

            if (frameCount == 10) {
                std::cout << "FPS = " << frameCount / sumDeltaTime << std::endl;
                sumDeltaTime = 0.0f;
                frameCount = 0;
            }
        }

        // bind g-buffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // copy buffer data (positions of particles)
        glBindVertexArray(particle.mesh.VAO);
        glBufferData(GL_ARRAY_BUFFER, simulator.numFluidParticles * sizeof(glm::vec3), &simulator.positions[0], GL_STATIC_DRAW);
        glBindVertexArray(0);

        // calculate depth
        shaderDepth.use();
        shaderDepth.setMat4("view", camera.getViewMatrix());
        shaderDepth.setMat4("projection", camera.getProjectionMatrix());

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        glBindVertexArray(particle.mesh.VAO);
        glDrawElementsInstanced(GL_TRIANGLES, particle.mesh.indices.size(), GL_UNSIGNED_INT, 0, simulator.numFluidParticles);

        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glBindVertexArray(0);

        // smooth depth
        shaderSmoothedDepth.use();
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);

        bool fromAtoB = true;
        bool firstSmoothing = true;

        for (int i = 0; i < numSmoothing; ++i) {
            shaderSmoothedDepth.setBool("fromAtoB", fromAtoB);
            glBindTexture(GL_TEXTURE_2D, firstSmoothing ? gTextureDepth : fromAtoB ? gTextureSmoothedDepthA : gTextureSmoothedDepthB);

            screenTriangle.draw(shaderSmoothedDepth);

            fromAtoB = !fromAtoB;
            if (firstSmoothing)
                firstSmoothing = false;
        }

        // calculate thickness
        shaderThickness.use();
        shaderThickness.setMat4("view", camera.getViewMatrix());
        shaderThickness.setMat4("projection", camera.getProjectionMatrix());

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glBindVertexArray(particle.mesh.VAO);
        glDrawElementsInstanced(GL_TRIANGLES, particle.mesh.indices.size(), GL_UNSIGNED_INT, 0, simulator.numFluidParticles);

        glDisable(GL_BLEND);
        glBindVertexArray(0);

        // restore normal
        shaderNormal.use();
        shaderNormal.setFloat("C", -2.0f * camera.tanHalfFovy / screenHeight);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fromAtoB ? gTextureSmoothedDepthA : gTextureSmoothedDepthB);

        screenTriangle.draw(shaderNormal);

        // copy the depth data from g-buffer to the scene framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sceneBuffer);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

        // bind the scene framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw the fluid
        shaderFluid.use();
        shaderFluid.setVec3("cameraPosition", camera.position);
        shaderFluid.setMat4("viewInv", glm::inverse(camera.getViewMatrix()));
        shaderFluid.setFloat("tanHalfFovy", camera.tanHalfFovy);
        shaderFluid.setFloat("aspect", camera.aspect);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapSkybox);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fromAtoB ? gTextureSmoothedDepthA : gTextureSmoothedDepthB);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gTextureThickness);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gTextureNormal);

        screenTriangle.draw(shaderFluid);

        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);

        // draw the floor
        shaderFloor.use();
        shaderFloor.setMat4("model", floor.getModelMatrix(glm::vec3(0.0f), glm::vec3(floorSize)));
        shaderFloor.setMat4("view", camera.getViewMatrix());
        shaderFloor.setMat4("projection", camera.getProjectionMatrix());

        floor.draw(shaderFloor);

        // draw the skybox
        shaderSkybox.use();
        shaderSkybox.setMat4("view", camera.getViewMatrix());
        shaderSkybox.setMat4("projection", camera.getProjectionMatrix());

        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapSkybox);

        skybox.draw(shaderSkybox);

        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);

        // bind the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw the screen quad
        shaderScreen.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gTextureDepth);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fromAtoB ? gTextureSmoothedDepthA : gTextureSmoothedDepthB);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gTextureThickness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gTextureNormal);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, sceneTexture);

        shaderScreen.setInt("displayMode", static_cast<int>(displayMode));
        screenTriangle.draw(shaderScreen);

        glfwSwapBuffers(window); // swap the buffers
        glfwPollEvents(); // poll events

        simulator.simulate();
    }

    // clear
    cube.clear();
    plane.clear();
    sphere.clear();
    ndcTriangle.clear();

    shaderDepth.clear();
    shaderSmoothedDepth.clear();
    shaderThickness.clear();
    shaderNormal.clear();
    shaderFluid.clear();
    shaderFloor.clear();
    shaderSkybox.clear();
    shaderScreen.clear();

    glfwTerminate();
    return 0;
}

void processKeyboardInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardMove(CameraMoveDir::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardMove(CameraMoveDir::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.processKeyboardMove(CameraMoveDir::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.processKeyboardMove(CameraMoveDir::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardMove(CameraMoveDir::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardMove(CameraMoveDir::BACKWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        if (!pauseKeyPressed) {
            pauseKeyPressed = true;
            simulator.pause();
            startTime = glfwGetTime();
        }
    } else
        pauseKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!resetKeyPressed) {
            resetKeyPressed = true;
            simulator.reset();
        }
    } else
        resetKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fixCameraKeyPressed) {
            fixCameraKeyPressed = true;
            camera.fix();
        }
    } else
        fixCameraKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        if (!displayDefaultKeyPressed) {
            displayDefaultKeyPressed = true;
            displayMode = DisplayMode::DEFAULT;
        }
    } else
        displayDefaultKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        if (!displayDepth0KeyPressed) {
            displayDepth0KeyPressed = true;
            displayMode = DisplayMode::DEPTH;
        }
    } else
        displayDepth0KeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        if (!displayDepth1KeyPressed) {
            displayDepth1KeyPressed = true;
            displayMode = DisplayMode::SMOOTHED_DEPTH;
        }
    } else
        displayDepth1KeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        if (!displayThicknessKeyPressed) {
            displayThicknessKeyPressed = true;
            displayMode = DisplayMode::THICKNESS;
        }
    } else
        displayThicknessKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        if (!displayNormalKeyPressed) {
            displayNormalKeyPressed = true;
            displayMode = DisplayMode::NORMAL;
        }
    } else
        displayNormalKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if (!displaySceneKeyPressed) {
            displaySceneKeyPressed = true;
            displayMode = DisplayMode::SCENE;
        }
    } else
        displaySceneKeyPressed = false;
}

void processMouseMove(GLFWwindow *window, double mouseX, double mouseY) {
    if (firstMouse) {
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        firstMouse = false;
    }

    float deltaX = mouseX - lastMouseX;
    float deltaY = lastMouseY - mouseY;

    lastMouseX = mouseX;
    lastMouseY = mouseY;

    camera.processMouseMove(deltaX, deltaY);
}

void processMouseScroll(GLFWwindow *window, double deltaX, double deltaY) {
    camera.processMouseScroll(deltaY);
}

unsigned int loadCubemap(const std::string &path, const std::string &type, bool gammaCorrection) {
    const std::vector<std::string> faces = { "right", "left", "top", "bottom", "front", "back" };
    unsigned int cubemap;
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    int width, height, numComponents;
    for (int i = 0; i < 6; ++i) {
        const std::string file = path + "/" + faces[i] + "." + type;
        unsigned char *data = stbi_load(file.c_str(), &width, &height, &numComponents, 0);
        if (data) {
            if (numComponents < 3)
                std::cout << "ERROR: Found cubemap texture with less than 3 components from file: " << path << std::endl;
            else {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gammaCorrection ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }
        } else
            std::cout << "ERROR: Failed to load cubemap texture from file: " << file << std::endl;

        stbi_image_free(data);
    }

    return cubemap;
}

unsigned int loadTexture(const std::string &path, bool gammaCorrection) {
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, numComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &numComponents, 0);
    if (data) {
        switch (numComponents) {
            case 1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                break;
            case 2:
                std::cout << "ERROR: Found texture with 2 components from file: " << path << std::endl;
                break;
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, gammaCorrection ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                break;
            default:
                glTexImage2D(GL_TEXTURE_2D, 0, gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
        }

        if (numComponents != 2) {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

    } else
        std::cout << "ERROR: Failed to load texture from file: " << path << std::endl;

    stbi_image_free(data);

    stbi_set_flip_vertically_on_load(false);
    return texture;
}