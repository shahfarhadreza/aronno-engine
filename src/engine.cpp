#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "glsystem.h"
#include "world.h"

Engine* Engine::sEngine = nullptr;

Engine::Engine(GLFWwindow* windowHandle)
    : mWindowHandle(windowHandle), mRenderer(nullptr), mResourceManager(nullptr), mWorld(nullptr) {
    if (Engine::sEngine != nullptr) {
        assert(0);
    }
    Engine::sEngine = this;
}

Engine::~Engine() {
    if (mWorld) {
        delete mWorld;
    }
    if (mResourceManager)
        delete mResourceManager;
    if (mRenderer)
        delete mRenderer;
}

bool Engine::init(enum RenderingSystem system) {
    if (system == RS_OPENGL) {
        mRenderer = new OpenGLRenderer(mWindowHandle);
    } else {
        printf("Error: Rendering System (%d) not supported.\n", system);
        return false;
    }
    if (!mRenderer->init()) {
        printf("Error: Failed to initialize Rendering System (%d)\n", system);
        return false;
    }
    mResourceManager = new ResourceManager();
    mWorld = new World();
    return true;
}

Renderer* Engine::getRenderingSystem() {
    return mRenderer;
}



