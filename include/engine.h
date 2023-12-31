#pragma once

using Entity_T = uint32_t;

class Renderer;
class ResourceManager;
class Light;
class DirectionalLight;
class PointLight;
class FrameBuffer;
class World;
class SceneEntity;
class CameraEntity;
class Texture;

#include "coremath.h"

enum RenderingSystem {
    RS_INVALID,
    RS_OPENGL,
    RS_VULKAN,
    RS_DIRECTX_11,
    RS_DIRECTX_12,
};

class Engine
{
private:
    static Engine* sEngine;
protected:
    GLFWwindow* mWindowHandle;
    Renderer* mRenderer;
    ResourceManager* mResourceManager;
    World* mWorld;
public:
    Engine(GLFWwindow* windowHandle);
    virtual ~Engine();

    bool init(enum RenderingSystem system);

    static Engine* get() { return sEngine; }

    World* getWorld() { return mWorld; }
    Renderer* getRenderingSystem();
    ResourceManager* getResourceManager() { return mResourceManager; }
    GLFWwindow* getWindowHandle() { return mWindowHandle; }
};



