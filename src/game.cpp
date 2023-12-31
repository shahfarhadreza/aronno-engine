#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"

#include "glsystem.h"

#include "game.h"

float projectileSize = 5;

Vertex projectileVertices[] = {
    // positions        // texture Coords
    Vertex(-projectileSize,    projectileSize, 0.0f, 0.0f, 1.0f),
    Vertex(-projectileSize,  -projectileSize, 0.0f, 0.0f, 0.0f),
    Vertex(projectileSize,    projectileSize, 0.0f, 1.0f, 1.0f),
    Vertex(projectileSize,  -projectileSize, 0.0f, 1.0f, 0.0f),
};

Game* Game::sStatic = nullptr;

Game::Game(GLFWwindow* window)
    : mWindow(window), mEngine(nullptr), mRend(nullptr),
    mResourceMgr(nullptr), mCurrentState(nullptr), mBindConstBuffers(true) {

    if (Game::sStatic != nullptr) {
        assert(0);
    }
    Game::sStatic = this;

    mMissionComplete = false;
}

Game::~Game() {
    for(DemonBase* d : mEnemyCharacterList) {
        delete d;
    }
    delete mPlayerCharacter;

    delete mCollisionMgr;

    if (mEngine)
        delete mEngine;
}

bool Game::loadResources() {
    std::cout << "Loading shaders..." << std::endl;
    depthProgram = mResourceMgr->loadShaders("shaders/glsl/depth.vert", "shaders/glsl/depth.frag");
    shadowDepthProgram = mResourceMgr->loadShaders("shaders/glsl/shadow_depth.vert", "shaders/glsl/shadow_depth.frag");
    shadowCubeDepthProgram = mResourceMgr->loadShaders("shaders/glsl/depth_cube.vert", "shaders/glsl/depth_cube.frag", "shaders/glsl/depth_cube.geom");
    lightProgram = mResourceMgr->loadShaders("shaders/glsl/lighting.vert", "shaders/glsl/lighting.frag");
    blurProgram = mResourceMgr->loadShaders("shaders/glsl/blurpass.vert", "shaders/glsl/blurpass.frag");
    ssaoProgram = mResourceMgr->loadShaders("shaders/glsl/ssao.vert", "shaders/glsl/ssao.frag");
    quadProgram = mResourceMgr->loadShaders("shaders/glsl/quad.vert", "shaders/glsl/quad.frag");
    hudProgram = mResourceMgr->loadShaders("shaders/glsl/hud.vert", "shaders/glsl/hud.frag");
    projectileProgram = mResourceMgr->loadShaders("shaders/glsl/projectile.vert", "shaders/glsl/projectile.frag");
    fxProgram = mResourceMgr->loadShaders("shaders/glsl/fx.vert", "shaders/glsl/fx.frag");
    skyProgram = mResourceMgr->loadShaders("shaders/glsl/skybox.vert", "shaders/glsl/skybox.frag");
    //sunProgram = mResourceMgr->loadShaders("shaders/glsl/sun.vert", "shaders/glsl/sun.frag", "shaders/glsl/sun.geom");

    std::cout << "Loading level..." << std::endl;
/*
    mDoorMesh = mResourceMgr->loadMesh("door_white.obj", "door", true);
    mDumpsterMesh = mResourceMgr->loadMesh("dumpster.obj", "dumpster", true);
    mDumpsterLidMesh = mResourceMgr->loadMesh("dumpster_lid.obj", "dumpster_lid", true);
*/
    mLevelMesh = mResourceMgr->loadMesh("plane.obj", "area_02", true);
    mPlayerMesh = mResourceMgr->loadSkeletonMesh("fps_animations_fn_502_tactical/scene.gltf", "fps_hand");
    //mDemonMesh = mResourceMgr->loadMesh("crate.obj", "demon");
/*
    mDoorInteractTex = mResourceMgr->loadTexture("textures/interact_door.png", true);
    mDumpsterInteractTex = mResourceMgr->loadTexture("textures/interact_dumpster.png", true);
*/
    mCrosshairTexture = mResourceMgr->loadTexture("textures/cross.png", true)->getGPUResource();
    mHudHealthTexture = mResourceMgr->loadTexture("textures/health_igi.png", true)->getGPUResource();
    mHudBulletTexture = mResourceMgr->loadTexture("textures/hud_bullet_igi.png", true)->getGPUResource();
    mHudFontTexture = mResourceMgr->loadTexture("textures/font.tga", true)->getGPUResource();
    mProjectileTexture = mResourceMgr->loadTexture("textures/blue_glow.png", true)->getGPUResource();
    mMuzzleTexture = mResourceMgr->loadTexture("textures/muzzleflash_fire.png", true);
    mBulletDecalTexture = mResourceMgr->loadTexture("textures/bullet_hole_0.png", true);

    mSkyTexture = mResourceMgr->loadCubeMapTexture({
                                    "./textures/sky/right.jpg",
                                    "./textures/sky/left.jpg",
                                    "./textures/sky/top.jpg",
                                    "./textures/sky/bottom.jpg",
                                    "./textures/sky/front.jpg",
                                    "./textures/sky/back.jpg"
                                   }, true);
    return true;
}

bool Game::init() {
    if (!mWindow) {
        std::cout << "ERROR: Window must be created before initializing Aronno Engine." << std::endl;
        std::cout << "ERROR: Failed to initialize Aronno Engine." << std::endl;
        return false;
    }
    std::cout << "Initializing engine..." << std::endl;
    mEngine = new Engine(mWindow);

    if (!mEngine->init(RS_OPENGL)) {
        std::cout << "ERROR: Failed to initialize Aronno Engine with OpenGL" << std::endl;
        return false;
    }

    mRend = mEngine->getRenderingSystem();
    mResourceMgr = mEngine->getResourceManager();
    mWorld = mEngine->getWorld();

    mCamera = mWorld->createCamera("PrimaryCamera");
    glm::quat qt = glm::quat(glm::vec3(0, 0, 0));
    mCameraTransform = &mWorld->addTransformComponent(mCamera, {0, 30, 0}, qt, {1, 1, 1});
    mCamera->mTransform = mCameraTransform;

    mWorld->setViewTarget(mCamera);

    mSunLight = mWorld->createSunLight(true);

    //mSunLight->setPosition(glm::vec3(-140.0f, 200.0f, 100.0f));
    mSunLight->setDirection(glm::vec3(1.0f, -1.0f, -0.45f));

    std::cout << "Initializing Bullet Physics Engine..." << std::endl;
    initPhysicsEngine();

    if (!loadResources()) {
        std::cout << "ERROR: Failed to load resources" << std::endl;
        return false;
    }

    initDynamicObjects();

    // load the map
    loadMap("plane.map");

    // buffers for screen quad
    mScreenQuad = mRend->createQuadBufferIndexed();
    mHUDQuad = mRend->createQuadBufferIndexed();
    mMuzzleQuad = mRend->createQuadBufferIndexed();

    {
        uint32_t vCount = sizeof(projectileVertices) / sizeof(Vertex);
        mProjectileVB = mRend->createGPUVertexBuffer(projectileVertices, vCount);

        uint32_t iCount = sizeof(quadIndices) / sizeof(uint32_t);
        mProjectileIB = mRend->createGPUIndexBuffer(quadIndices, iCount);
    }

    {
        Vertex vertices[] = {
            Vertex(-0.5, -0.5, -0.5, 1.0,1.0),
            Vertex(0.5, -0.5, -0.5, 1.0,1.0),
            Vertex(0.5,  0.5, -0.5, 1.0,1.0),
            Vertex(-0.5,  0.5, -0.5, 1.0,1.0),
            Vertex(-0.5, -0.5,  0.5, 1.0,1.0),
            Vertex(0.5, -0.5,  0.5, 1.0,1.0),
            Vertex(0.5,  0.5,  0.5, 1.0,1.0),
            Vertex(-0.5,  0.5,  0.5, 1.0,1.0),
        };

        uint32_t elements[] = {
            0, 1, 2, 3,
            4, 5, 6, 7,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        uint32_t vCount = sizeof(vertices) / sizeof(Vertex);
        mBoundingBoxVB = mRend->createGPUVertexBuffer(vertices, vCount);

        uint32_t iCount = sizeof(elements) / sizeof(uint32_t);
        mBoundingBoxIB = mRend->createGPUIndexBuffer(elements, iCount);
    }

    {
        Vertex vertices[] = {
            Vertex(-0.5, 0, -0.5, 1.0,1.0),
            Vertex(0.5, 0, -0.5, 1.0,1.0),
            Vertex(0.2,  1, -0.2, 1.0,1.0),
            Vertex(-0.2,  1, -0.2, 1.0,1.0),
            Vertex(-0.5, 0,  0.5, 1.0,1.0),
            Vertex(0.5, 0,  0.5, 1.0,1.0),
            Vertex(0.2,  1,  0.2, 1.0,1.0),
            Vertex(-0.2,  1,  0.2, 1.0,1.0),
        };

        uint32_t elements[] = {
            0, 1, 2, 3,
            4, 5, 6, 7,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        uint32_t vCount = sizeof(vertices) / sizeof(Vertex);
        mBoneVB = mRend->createGPUVertexBuffer(vertices, vCount);

        uint32_t iCount = sizeof(elements) / sizeof(uint32_t);
        mBoneIB = mRend->createGPUIndexBuffer(elements, iCount);
    }

    {
        // max 20 vertices will be enough!
        mHUDTextHealthVB = mRend->createGPUVertexBuffer(nullptr, 64);
        mHUDTextAmmoVB = mRend->createGPUVertexBuffer(nullptr, 64);
        mHUDMsgTextVB = mRend->createGPUVertexBuffer(nullptr, 256);
    }

    // SkyBox
    {
        Vertex vertices[] = {
            // positions
            Vertex(-1.0f,  1.0f, -1.0f),
            Vertex(-1.0f, -1.0f, -1.0f),
             Vertex(1.0f, -1.0f, -1.0f),
             Vertex(1.0f, -1.0f, -1.0f),
             Vertex(1.0f,  1.0f, -1.0f),
            Vertex(-1.0f,  1.0f, -1.0f),

            Vertex(-1.0f, -1.0f,  1.0f),
            Vertex(-1.0f, -1.0f, -1.0f),
            Vertex(-1.0f,  1.0f, -1.0f),
           Vertex( -1.0f,  1.0f, -1.0f),
           Vertex( -1.0f,  1.0f,  1.0f),
          Vertex(  -1.0f, -1.0f,  1.0f),

            Vertex( 1.0f, -1.0f, -1.0f),
           Vertex(  1.0f, -1.0f,  1.0f),
           Vertex(  1.0f,  1.0f,  1.0f),
           Vertex(  1.0f,  1.0f,  1.0f),
          Vertex(   1.0f,  1.0f, -1.0f),
          Vertex(   1.0f, -1.0f, -1.0f),

          Vertex(  -1.0f, -1.0f,  1.0f),
          Vertex(  -1.0f,  1.0f,  1.0f),
          Vertex(   1.0f,  1.0f,  1.0f),
          Vertex(   1.0f,  1.0f,  1.0f),
          Vertex(   1.0f, -1.0f,  1.0f),
          Vertex(  -1.0f, -1.0f,  1.0f),

          Vertex(  -1.0f,  1.0f, -1.0f),
          Vertex(   1.0f,  1.0f, -1.0f),
          Vertex(   1.0f,  1.0f,  1.0f),
          Vertex(   1.0f,  1.0f,  1.0f),
          Vertex(  -1.0f,  1.0f,  1.0f),
          Vertex(  -1.0f,  1.0f, -1.0f),

          Vertex(  -1.0f, -1.0f, -1.0f),
          Vertex(  -1.0f, -1.0f,  1.0f),
          Vertex(   1.0f, -1.0f, -1.0f),
          Vertex(   1.0f, -1.0f, -1.0f),
          Vertex(  -1.0f, -1.0f,  1.0f),
          Vertex(   1.0f, -1.0f,  1.0f)
        };

        // buffers for cube
        uint32_t vCount = sizeof(vertices) / sizeof(Vertex);
        mSkyBoxVB = mRend->createGPUVertexBuffer(vertices, vCount);
    }

    mCBPerFrame = mRend->createGPUConstantBuffer(sizeof(cbPerFrame));
    mCBPerObject = mRend->createGPUConstantBuffer(sizeof(cbPerObject));
    mCBPerAnimatedObject = mRend->createGPUConstantBuffer(sizeof(cbPerAnimatedObject));
    mCBLightArray = mRend->createGPUConstantBuffer(sizeof(cbLightArray));
    mCBPostProcess = mRend->createGPUConstantBuffer(sizeof(cbPostProcess));
    mCBCascadedShadow = mRend->createGPUConstantBuffer(sizeof(cbCascadedShadow));
    mCBCascadedShadowProj = mRend->createGPUConstantBuffer(sizeof(cbCascadedShadowProj));
    mCBShadowCube = mRend->createGPUConstantBuffer(sizeof(cbShadowCube));

    mPerFrameData.enableSSAO = 1;
    mPerFrameData.SSAODistance = 1.0f;
    mPerFrameData.SSAODistancePower = 0.950f;
    mPerFrameData.SSAOMoveAmount = 2.5f;

    mPerFrameData.sunEnableShadow = 1;
    mPerFrameData.sunlightIntensity = 15.0f;
    mPerFrameData.sunlightAmbient = 0.050f;
    mPerFrameData.sunIndirectIntensity = 0.005f;

    mPerFrameData.postEnableBloom = 1;
    mPerFrameData.postSaturation = 1.2f;
    mPerFrameData.postBloomIntensity = 0.15f;
    mPerFrameData.postExposure = 1.5;
    mPerFrameData.postbBrightMax = 20.0;
    mPerFrameData.postEnableToneMapping = 1;

    mPerFrameData.fogDensity = 0.001f;
    mPerFrameData.metallic = 0.0f;
    mPerFrameData.roughness = 0.0f;

    const uint32_t numSamples = 32;

    struct {
	    glm::vec4 samples[numSamples];
	} SSAOData;

    mCBSSAO = mRend->createGPUConstantBuffer(sizeof(SSAOData));

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    for (unsigned int i = 0; i < numSamples; ++i)
    {
        glm::vec4 sample(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator),
            0.0f
        );
        sample  = glm::normalize(sample);
        sample *= randomFloats(generator);

        float scale = (float)i / float(numSamples);
        scale   = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        SSAOData.samples[i] = sample;
    }

    mCBSSAO->updateData(&SSAOData);

    const int noiseW = 4;
    const int noiseH = 4;
    const int noiseSize = noiseW * noiseH;

    std::vector<glm::vec3> ssaoNoise;
    for (int i = 0; i < noiseSize; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        ssaoNoise.push_back(noise);
    }

    mNoiseTexture = mRend->createGPUTexture(noiseW, noiseH, &ssaoNoise[0], TextureFormat::RGBA32_FLOAT);


    std::cout << "Initializing frame buffer objects..." << std::endl;
    initFrameBuffers();

    std::cout << "Starting..." << std::endl;
    mCurrentState = new GamePlayState();
    mCurrentState->start(nullptr);

    return true;
}

void Game::initFrameBuffers() {

    // Depth Buffer (SSAO)
    FrameBufferDesc smDesc;
    smDesc.Width = SCR_WIDTH;
    smDesc.Height = SCR_HEIGHT;
    smDesc.Flags = FRAME_BUFFER_FLAG_DEPTH;
    smDesc.FilterType = TextureFilterType::LinearFilter;
    smDesc.NumRenderTarget = 0;

    depthFBO = mRend->createFrameBufferObject(smDesc);

    // Shadow Map/Depth Buffer
    smDesc.Flags = FRAME_BUFFER_FLAG_SHADOW;
    smDesc.FilterType = TextureFilterType::LinearFilter;
    smDesc.NumRenderTarget = 0;

    smDesc.Width = 4096;
    smDesc.Height = 4096;
    mCascadedFBOSplit1 = mRend->createFrameBufferObject(smDesc);

    smDesc.Width = 3048;
    smDesc.Height = 3048;
    mCascadedFBOSplit2 = mRend->createFrameBufferObject(smDesc);

    smDesc.Width = 4096;
    smDesc.Height = 4096;
    mCascadedFBOSplit3 = mRend->createFrameBufferObject(smDesc);

    // Primary Buffer
    FrameBufferDesc pDesc;
    pDesc.Width = SCR_WIDTH;
    pDesc.Height = SCR_HEIGHT;
    pDesc.Flags = FRAME_BUFFER_FLAG_COLOR | FRAME_BUFFER_FLAG_DEPTH_STENCIL;
    pDesc.FilterType = TextureFilterType::LinearFilter;
    pDesc.NumRenderTarget = 2;

    pDesc.RenderTargetDescList = {
        RenderTargetDesc {
            .FilterType = TextureFilterType::LinearFilter,
            .Format = TextureFormat::RGBA32_FLOAT
        },
        RenderTargetDesc {
            .FilterType = TextureFilterType::LinearFilter,
            .Format = TextureFormat::RGBA32_FLOAT
        },
    };
    primaryFBO = mRend->createFrameBufferObject(pDesc);

    // Blur Pass Buffer
	FrameBufferDesc blurDesc;
	blurDesc.Width = SCR_WIDTH / 2;
	blurDesc.Height = SCR_HEIGHT / 2;
	blurDesc.Flags = FRAME_BUFFER_FLAG_COLOR;
	blurDesc.FilterType = TextureFilterType::LinearFilter;
	blurDesc.NumRenderTarget = 1;

	blurDesc.RenderTargetDescList = {
	    RenderTargetDesc {
	        .FilterType = TextureFilterType::LinearFilter,
	        .Format = TextureFormat::RGBA32_FLOAT
	    }
	};

	blurPassFBO1 = mRend->createFrameBufferObject(blurDesc);
	blurPassFBO2 = mRend->createFrameBufferObject(blurDesc);

	blurDesc.Width = SCR_WIDTH / 2;
	blurDesc.Height = SCR_HEIGHT / 2;
	blurDesc.Flags = FRAME_BUFFER_FLAG_COLOR;

	blurDesc.RenderTargetDescList = {
	    RenderTargetDesc {
	        .FilterType = TextureFilterType::LinearFilter,
	        .Format = TextureFormat::R32_FLOAT
	    }
	};

	ssaoFBO = mRend->createFrameBufferObject(blurDesc);
}

void Game::initPhysicsEngine() {
    mCollisionMgr = new CollisionManager();
}

void Game::initDynamicObjects() {
    // Setup entities

    mLevelEntity = mWorld->createEntity("static_level");
    mWorld->addMeshComponent(mLevelEntity, mLevelMesh);
    mCollisionMgr->registerStaticEntity(mLevelEntity);

    // Create player hands
    SceneEntity* playerHandsEntity = mWorld->createEntity("player_hands", mCamera);
    mWorld->addMeshComponent(playerHandsEntity, mPlayerMesh);

    glm::quat qt = glm::quat(glm::vec3(glm::radians(-8.0f), glm::radians(187.0f), 0));
    mWorld->addTransformComponent(playerHandsEntity, {2, -7, -5}, qt, {30, 30, 30});

    // Actual player
    mPlayerCharacter = new Player(mCollisionMgr->getDynamicsWorld(), glm::vec3(-20.0f, 80.0f, 135.5f), playerHandsEntity, mCamera);

    //demon = new DemonBase(mDemonMesh, dynamicsWorld, glm::vec3(50.0f, 120.0f, 0.5f));
    //mEnemyCharacterList.push_back(demon);

    //demon = new DemonBase(mDemonMesh, dynamicsWorld, glm::vec3(100.0f, 120.0f, 40.5f));
    //mEnemyCharacterList.push_back(demon);
}

float cameraTime = 0;

void Game::update(float dt) {
    if (mCurrentState) {
        mCurrentState->update(dt);
    }
    CameraEntity* cam = mWorld->getViewTarget();
    cam->updateProjection(float(SCR_WIDTH), float(SCR_HEIGHT));

    mCollisionMgr->update(dt);
    mSunLight->update(dt);
    mWorld->update(dt);

    btTransform trans = mPlayerCharacter->getGhostObject()->getWorldTransform();

    btVector3 vel = mPlayerCharacter->getController()->getLinearVelocity();
    btVector3 velXZ(vel.getX(), 0.0f, vel.getZ());

    float velocity = velXZ.length() * 5;

    float offset_factor = sin(cameraTime);

    glm::vec3 playerPos(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()) + 35 + offset_factor, float(trans.getOrigin().getZ()));

    mCameraTransform->Position = playerPos;

    // do bobbing math
    cameraTime += dt * 15 * velocity;

    if (cameraTime >= 6.2) {
        cameraTime = 0;
    }

    if (mPlayerCharacter) {
        mPlayerCharacter->update(dt);
    }

    mInteractionMgr.update(dt);

    // Update demons too!
    for(auto it = mEnemyCharacterList.begin(); it != mEnemyCharacterList.end();) {
        DemonBase* ch = *it;
        if (ch->isMarkedForRemove()) {
            ch->triggerRemove();
            it = mEnemyCharacterList.erase(it);
            delete ch;
        } else {
            ch->update(dt);
            it++;
        }
    }

    // Update decals
    for(auto it = mDecals.begin(); it != mDecals.end();) {
        Decal& decal = *it;

        if (decal.isMarkedForRemove()) {
            decal.triggerRemove();
            it = mDecals.erase(it);
        }
        else {
            decal.update(dt);
            ++it;
        }
    }

    // Update bullet projectiles
    for(auto it = mBulletProjectiles.begin(); it != mBulletProjectiles.end();) {

        BulletProjectile& bullet = *it;
        float distance = glm::distance(bullet.mOrigin, bullet.mPosition);

        if (distance > bullet.mMaxDistance || bullet.isMarkedForRemove()) {
            bullet.triggerRemove();
            it = mBulletProjectiles.erase(it);
        }
        else {
            bullet.update(dt);

            // If this bullet collides with any demon, do hit damage
            for(DemonBase* ch : mEnemyCharacterList) {

                AABB meshBBox = ch->mMesh->getBoundingBox();

                // Mesh local bbox to World space bbox
                meshBBox.transform(ch->mTransform);

                INTERSECTION_TYPE result = bullet.mWorldBoundingBox.intersect(meshBBox);

                if (result != INTERSECTION_TYPE::OUTSIDE) {
                    //printf("BULLET HIT!!!\n");
                    bullet.markForRemove();
                    ch->triggerDamage();
                }
            }
            ++it;
        }
    }

    // Check for mission complete
    if (mEnemyCharacterList.size() == 0 && mMissionComplete == false) {
        mPopupTextList.push_back(PopupText("Mission Complete", {(float(SCR_WIDTH) / 2) - 260, (float(SCR_HEIGHT) / 2) - 60}, 30, 300));
        mMissionComplete = true;
    }

    // Check for popups
    for(auto it = mPopupTextList.begin(); it != mPopupTextList.end();) {
        if ((*it).isMarkedForRemove()) {
            (*it).triggerRemove();
            it = mPopupTextList.erase(it);
        } else {
            (*it).update(dt);
            it++;
        }
    }
}



