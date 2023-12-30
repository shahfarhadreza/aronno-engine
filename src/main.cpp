#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "game.h"

// mouse variables
float lastXpos = (float)(SCR_WIDTH / 2);
float lastYpos = (float)(SCR_HEIGHT/ 2);
bool firstMouse = true;

Game* game = nullptr;

bool glfwSetWindowCenter( GLFWwindow * window );

bool mEditMode = true;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if (mEditMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        mEditMode = !mEditMode;
    } else {
        if (game) {
            game->keyboardEvent(key, scancode, action, mods);
        }
    }
}

void processMouseInput(GLFWwindow* window, double xPosition, double yPosition) {
	if (firstMouse) {
		lastXpos = xPosition;
		lastYpos = yPosition;
		firstMouse = false;
	}

	float mouseSensitivity = 0.1f;

	// calculating the offset
	float xOffset = xPosition - lastXpos;
	float yOffset = lastYpos - yPosition;

	// setting the current mouse position to the last mouse position
	lastXpos = xPosition;
	lastYpos = yPosition;

	// multiplying with the mouse sensitivity
	xOffset *= mouseSensitivity;
	yOffset *= mouseSensitivity;

	if (!mEditMode) {
        if (game) {
            game->mouseMoveEvent(xPosition, yPosition, xOffset, yOffset);
        }
	} else {
	    ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(xPosition, yPosition);
	}
}

void processMouseButton(GLFWwindow* window, int button, int action, int mods) {
    if (!mEditMode) {
        if (game) {
            game->mouseButtonEvent(button, action);
        }
    } else {
        // (1) ALWAYS forward mouse data to ImGui! This is automatic with default backends. With your own backend:
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    }
}

int main() {
	glfwInit();
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PROJECT IGI", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "ERROR::WINDOW::CREATION" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetWindowCenter(window);

	const char* glsl_version = "#version 460";

	std::cout << "Setting up Dear ImGui..." << std::endl;
	// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

	//glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
	glfwSetCursorPosCallback(window, processMouseInput);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, processMouseButton);

	game = new Game(window);

	if(!game->init()) {
        std::cout << "ERROR: Failed to start game" << std::endl;
		glfwTerminate();
		return -1;
	}

	//glEnable(GL_MULTISAMPLE);

	float lastFrame = 0.0f;

	const double dt = 0.01;

	double lastTime = glfwGetTime();
	double accumulator = 0.0;

    int nbFrames = 0;

    char buf[256];
    int nDrawCalls = 0;

	// game loop
	while (!glfwWindowShouldClose(window)) {

        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
            // printf and reset timer
            sprintf(buf, "DOOM - %d FPS, %f ms/frame, %d Draw Calls",
                    nbFrames, 1000.0/double(nbFrames), nDrawCalls);
            glfwSetWindowTitle(window, buf);
            nbFrames = 0;
            lastTime += 1.0;
        }
        nDrawCalls = 0;
        // Update

		// delta time
		float currentFrame = (float)glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		accumulator += deltaTime;

        while ( accumulator >= dt )
        {
            game->processKeyboardInput(dt);
            game->update(dt);
            accumulator -= dt;
        }

		game->render();

		glfwPollEvents();
	}

	delete game;

	// Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

