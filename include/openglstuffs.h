#pragma once

float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int loadCubemap(std::vector<std::string> faces);

/*
	Vertex vertices[] = {
	    Vertex(-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1, 1, 1),  // A 0
        Vertex(0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1, 1, 1),  // B 1
        Vertex(0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1, 1, 1),  // C 2
        Vertex(-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1, 1, 1),  // D 3
        Vertex(-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1, 1, 1),  // E 4
        Vertex(0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1, 1, 1),   // F 5
        Vertex(0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1, 1, 1),   // G 6
        Vertex(-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1, 1, 1),   // H 7

        Vertex(-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1, 1, 1),  // D 8
        Vertex(-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1, 1, 1),  // A 9
        Vertex(-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1, 1, 1),  // E 10
        Vertex(-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1, 1, 1),  // H 11
        Vertex(0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1, 1, 1),   // B 12
        Vertex(0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1, 1, 1),   // C 13
        Vertex(0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1, 1, 1),   // G 14
        Vertex(0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1, 1, 1),   // F 15

        Vertex(-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1, 1, 1),  // A 16
        Vertex(0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1, 1, 1),   // B 17
        Vertex(0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1, 1, 1),   // F 18
        Vertex(-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1, 1, 1),  // E 19
        Vertex(0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1, 1, 1),  // C 20
        Vertex(-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1, 1, 1),  // D 21
        Vertex(-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1, 1, 1),  // H 22
        Vertex(0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 1, 1, 1),  // G 23
	};

	// buffers for cube
    uint32_t vCount = sizeof(vertices) / sizeof(Vertex);
	IGPUResource* lightVBO = rnd->createGPUVertexBuffer(vertices, vCount);

	std::cout << "Loading shaders..." << std::endl;
	GLProgram* lightShaderProgram = LoadShaders("light.vert", "light.frag");
	// shader program
	GLProgram* primaryShaderProgram = LoadShaders("normal_map.vert", "normal_map.frag");//, "default.geom");
	GLProgram* depthProgram = LoadShaders("depth.vert", "depth.frag");
	//GLProgram* debugProgram = LoadShaders("debug_quad.vert", "debug_quad.frag");

	GLProgram* quadProgram = LoadShaders("quad.vert", "quad.frag");

	GLProgram* brightPassProgram = LoadShaders("brightpass.vert", "brightpass.frag");
	GLProgram* blurPassProgram = LoadShaders("blurpass.vert", "blurpass.frag");

	GLProgram* skyProgram = LoadShaders("skybox.vert", "skybox.frag");

	// setup plane VAO
	GLuint skyboxVAO;
	GLuint skyboxVBO;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	//glEnable(GL_FRAMEBUFFER_SRGB);

	rnd->setDepthTest(true);

	// Primary Buffer
	FrameBufferDesc pDesc;
	pDesc.Width = SCR_WIDTH;
	pDesc.Height = SCR_HEIGHT;
	pDesc.Flags = FRAME_BUFFER_FLAG_COLOR | FRAME_BUFFER_FLAG_DEPTH_STENCIL;

	GLFrameBuffer primaryFBO(pDesc);

	// BrightPass Buffer
	FrameBufferDesc bpDesc;
	bpDesc.Width = SCR_WIDTH;
	bpDesc.Height = SCR_HEIGHT;
	bpDesc.Flags = FRAME_BUFFER_FLAG_COLOR | FRAME_BUFFER_FLAG_DEPTH_STENCIL;

	GLFrameBuffer brightPassFBO(bpDesc);

	// HBlurPass Buffer
	FrameBufferDesc hblurDesc;
	hblurDesc.Width = SCR_WIDTH ;
	hblurDesc.Height = SCR_HEIGHT ;
	hblurDesc.Flags = FRAME_BUFFER_FLAG_COLOR | FRAME_BUFFER_FLAG_DEPTH_STENCIL;

	GLFrameBuffer hblurPassFBO(hblurDesc);

	// VBlurPass Buffer
	FrameBufferDesc vblurDesc;
	vblurDesc.Width = SCR_WIDTH ;
	vblurDesc.Height = SCR_HEIGHT ;
	vblurDesc.Flags = FRAME_BUFFER_FLAG_COLOR | FRAME_BUFFER_FLAG_DEPTH_STENCIL;

	GLFrameBuffer vblurPassFBO(vblurDesc);
*/



