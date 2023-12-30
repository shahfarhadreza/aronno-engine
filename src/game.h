#pragma once

// global variables
#define SCR_WIDTH 1600
#define SCR_HEIGHT 900

#include <regex>
inline std::string ltrim(std::string const &str) {
    return std::regex_replace(str, std::regex("^\\s+"), "");
}

inline std::string rtrim(std::string const &str) {
    return std::regex_replace(str, std::regex("\\s+$"), "");
}

inline std::string trim(std::string const &str) {
   return ltrim(rtrim(str));
}

class DynamicObject
{
public:
    Mesh* mMesh;
    btTriangleMesh* meshBullet;
    btCollisionShape* colShape;
    btDefaultMotionState* motionState;
    btRigidBody* rigidBody;
public:
    DynamicObject() {
    }
};

class Removable
{
public:
    bool mMarkedForRemove;
    Removable() : mMarkedForRemove(false) {
    }
    virtual ~Removable() { }
    virtual bool isMarkedForRemove() const { return mMarkedForRemove; }
    virtual void markForRemove() {
        mMarkedForRemove = true;
    }

    virtual void triggerRemove() { }
};

class Character : public Removable
{
public:
    btDiscreteDynamicsWorld* mDynamicsWorld;
    btConvexShape* mCollisionShape = nullptr;
    btDefaultMotionState* mMotionState = nullptr;
    btPairCachingGhostObject* mGhostObject = nullptr;
    btKinematicCharacterController* mController = nullptr;

    int mHealth;
    int mAmmo;
    int mClipAmmo;
    int mMaxClipAmmo;
public:
    Character(btDiscreteDynamicsWorld* dynamicsWorld, const glm::vec3& origin);
    virtual ~Character();

    btPairCachingGhostObject* getGhostObject() {
        return mGhostObject;
    }
    btKinematicCharacterController* getController() {
        return mController;
    }

    void setPosition(const glm::vec3& pos);

    int getHealth() const {
        return mHealth;
    }

    int getAmmo() const {
        return mAmmo;
    }

    int getClipAmmo() const {
        return mClipAmmo;
    }

    bool hasAmmo() const {
        return mAmmo > 0 || mClipAmmo > 0;
    }
};

class Player : public Character
{
public:
    SceneEntity* mPlayerEntity;
    CameraEntity* mCamera;
    TransformComponent* mHandTransform;
    TransformComponent* mMuzzleTransform;
    BillboardComponent* mMuzzle;
    PointLight* mMuzzleLight;
    float mAnimationTime;
    float mShakeAnimationTime;
    float mMuzzleScale;
    Skeleton* mSkeHands;
    AnimationState* mAnimIdle;
    AnimationState* mAnimShot;
    AnimationState* mAnimWalk;
    AnimationState* mAnimReload;
public:
    Player(btDiscreteDynamicsWorld* dynamicsWorld, const glm::vec3& origin, SceneEntity* entity, CameraEntity* camera);
    void triggerShoot();
    virtual void update(float dt);
};

class DemonBase : public Character
{
public:
    Mesh* mMesh;
    glm::mat4 mTransform;
public:
    DemonBase(Mesh* mesh, btDiscreteDynamicsWorld* dynamicsWorld, const glm::vec3& origin)
        : Character(dynamicsWorld, origin), mMesh(mesh) {
        mTransform = glm::mat4(1.0f);
    }

    virtual void triggerDamage();

    virtual void update(float dt);
};

struct cbPerFrame {
    glm::mat4 proj;
    glm::mat4 projInverse;
    glm::mat4 view;
    glm::mat4 viewRotation;
    glm::vec4 cameraPosition;
    int screenWidth;
    int screenHeight;
    float cameraNear;
    float cameraFar;

    int enableSSAO;
    float SSAODistance;
    float SSAODistancePower;
    float SSAOMoveAmount;

    glm::vec3 sunDirection;
    float sunlightBias;

    float sunlightIntensity;
    float sunlightAmbient;
    float sunIndirectIntensity;
    int sunEnableShadow;

    int postEnableBloom;
    int postEnableToneMapping;
    float postSaturation;
    float postBloomIntensity;

    float postExposure;
    float postbBrightMax;
    float fogDensity;
    float metallic;

    float roughness;
    float pad1;
    float pad2;
    float pad3;
};

struct cbPerObject {
    glm::mat4 world;
    float hasNormalMap;
    float hasEmissionMap;
    float opacity;
    float specularIntensity;
    float specularGlossiness;
    float emissionIntensity;
    float animated;
    float isTransparent;
};

struct cbPerAnimatedObject {
    glm::mat4 gBones[MAX_BONES];
};

struct cbPointLight {
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 color;
};

struct cbLightArray {
    glm::vec4 lightCount;
    cbPointLight lights[16];
};

struct cbShadowCube {
    glm::mat4 shadowMatrices[6];
    glm::vec4 lightPos;
};

struct cbPostProcess {
    float horizontalPass;
    glm::vec3 pad;
};

const int MAX_CSSM_SPLITS = 3;

struct cbCascadedShadow {
    glm::mat4 splits[MAX_CSSM_SPLITS];
    glm::vec4 cascadePlaneDistances;
};

struct cbCascadedShadowProj {
    glm::mat4 lightProjView;
};

class GameState
{
public:
    GameState() {
    }

    virtual void start(GameState* prev) { }

    virtual void update(float dt) {

    }

    virtual void render() {

    }
};

class GamePlayState : public GameState
{
public:
    GamePlayState() {
    }

    virtual void start(GameState* prev) override;
    virtual void update(float dt) override;
    virtual void render() override;
};

class BulletProjectile : public Removable
{
public:
    glm::vec3 mOrigin;
    glm::vec3 mDirection;
    glm::vec3 mPosition;
    float mSpeed;
    float mMaxDistance;
    AABB mLocalBoundingBox;
    AABB mWorldBoundingBox;
    glm::mat4 mTransform;
public:
    BulletProjectile(const glm::vec3& origin, const glm::vec3& dir);
    void update(float dt);
};

class PopupText : public Removable
{
public:
    std::string mText;
    glm::vec2 mOrigin;
    float mSize;
    float mDuration;
    float mTime;
public:
    PopupText(const std::string& text, const glm::vec2& origin, float size, float duration)
        : mText(text), mOrigin(origin), mSize(size), mDuration(duration), mTime(0) {
    }
    ~PopupText() {
    }
    void update(float dt);
};

class Decal : public Removable
{
public:
    glm::vec3 mOrigin;
    glm::vec3 mDirection;
    glm::mat4 mTransform;
public:
    Decal(const glm::vec3& origin, const glm::vec3& dir);
    void update(float dt);
};

class CollisionObject
{
public:
    btCollisionShape* mColShape;
    btDefaultMotionState* mMotionState;
    btRigidBody* mRigidBody;
    btTransform mTransform;
    CollisionObject();
    ~CollisionObject();
};

class CollisionManager
{
protected:
    // Bullet Physics
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* overlappingPairCache;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    std::unordered_map<SceneEntity*, CollisionObject*> mCollisionObjects;
public:
    CollisionManager();
    ~CollisionManager();

    btDiscreteDynamicsWorld* getDynamicsWorld() { return dynamicsWorld; }

    void registerStaticEntity(SceneEntity* entity);

    void update(float dt);
};

class InteractionManager
{
private:
    static InteractionManager* sStatic;
public:
    SceneEntity* mLastObject;
    SceneEntity* mActiveObject;
    InteractComponent* mComponent;
    float mAnimationTime;
public:
    InteractionManager();
    ~InteractionManager() { }

    static InteractionManager* get() { return sStatic; }

    void update(float dt);
};

enum RenderPassType {
    DepthPass,
    SunShadowPass,
    PointShadowPass,
    LightingPass
};

class Game
{
private:
    static Game* sStatic;
public:
    static Game* get() { return sStatic; }
public:
    GLFWwindow* mWindow;
    Engine* mEngine;
    Renderer* mRend;
    CollisionManager* mCollisionMgr;
    ResourceManager* mResourceMgr;
    InteractionManager mInteractionMgr;
    World* mWorld;
    DirectionalLight* mSunLight;
    GameState* mCurrentState;
    CameraEntity* mCamera;
    TransformComponent* mCameraTransform;
    bool mBindConstBuffers;

    char debugText[512];

    Player* mPlayerCharacter;
    std::vector<DemonBase*> mEnemyCharacterList;
    std::vector<BulletProjectile> mBulletProjectiles;
    std::vector<Decal> mDecals;
    std::vector<PopupText> mPopupTextList;

    bool mMissionComplete;

    Mesh* mLevelMesh;
    SkeletonMesh* mPlayerMesh;
    Mesh* mDemonMesh;
    Mesh* mDoorMesh;
    Mesh* mDumpsterMesh;
    Mesh* mDumpsterLidMesh;

    SceneEntity* mLevelEntity;

    DynamicObject mCrate;

    cbPerFrame mPerFrameData;
    cbPerObject mPerObjectData;
    cbCascadedShadow mCascadedShadowData;
    cbPerAnimatedObject mPerAnimatedObjectData;
    cbLightArray mLightArrayData;
    cbPostProcess mPostProcessData;

    QuadBufferIndexed* mScreenQuad;
    QuadBufferIndexed* mHUDQuad;

    IGPUVertexBuffer* mHUDTextHealthVB;
    IGPUVertexBuffer* mHUDTextAmmoVB;
    IGPUVertexBuffer* mHUDMsgTextVB;

    IGPUVertexBuffer* mProjectileVB;
    IGPUIndexBuffer* mProjectileIB;

    IGPUVertexBuffer* mBoundingBoxVB;
    IGPUIndexBuffer* mBoundingBoxIB;

    IGPUVertexBuffer* mBoneVB;
    IGPUIndexBuffer* mBoneIB;

    IGPUVertexBuffer* mSkyBoxVB;

    QuadBufferIndexed* mMuzzleQuad;
// Shaders:
    IGPUShaderProgram* depthProgram;
    IGPUShaderProgram* shadowDepthProgram;
    IGPUShaderProgram* shadowCubeDepthProgram;
    IGPUShaderProgram* lightProgram;
    IGPUShaderProgram* ssaoProgram;
    IGPUShaderProgram* blurProgram;
    IGPUShaderProgram* quadProgram;
    IGPUShaderProgram* hudProgram;
    IGPUShaderProgram* projectileProgram;
    IGPUShaderProgram* fxProgram;
    IGPUShaderProgram* skyProgram;
    IGPUShaderProgram* sunProgram;
// Textures:
    IGPUTexture* mNoiseTexture;
    IGPUTexture* mCrosshairTexture;
    IGPUTexture* mHudHealthTexture;
    IGPUTexture* mHudBulletTexture;
    IGPUTexture* mHudFontTexture;
    IGPUTexture* mProjectileTexture;
    Texture* mBulletDecalTexture;
    Texture* mMuzzleTexture;
    Texture* mDoorInteractTex;
    Texture* mDumpsterInteractTex;
    Texture* mSkyTexture;
// Buffers:
    IGPUConstantBuffer* mCBPerFrame;
    IGPUConstantBuffer* mCBPerObject;
    IGPUConstantBuffer* mCBCascadedShadow;
    IGPUConstantBuffer* mCBCascadedShadowProj;
    IGPUConstantBuffer* mCBPerAnimatedObject;
    IGPUConstantBuffer* mCBLightArray;
    IGPUConstantBuffer* mCBShadowCube;
    IGPUConstantBuffer* mCBPostProcess;
    IGPUConstantBuffer* mCBSSAO;

    FrameBuffer* depthFBO;
    FrameBuffer* mCascadedFBOSplit1;
    FrameBuffer* mCascadedFBOSplit2;
    FrameBuffer* mCascadedFBOSplit3;
    FrameBuffer* primaryFBO;
    FrameBuffer* ssaoFBO;
    FrameBuffer* blurPassFBO1;
    FrameBuffer* blurPassFBO2;
public:
    Game(GLFWwindow* window);
    ~Game();

    bool init();
    void initPhysicsEngine();
    void initFrameBuffers();
    bool loadResources();
    bool loadMap(const std::string& filename);
    void initDynamicObjects();
    void update(float dt);
    void _preparePerFrameData();
    void _prepareLightData();
    void _bindShaders();
    void renderScene(enum RenderPassType pass, Frustum* frustum);
    void render();
    void renderHUD();
    void renderGUI();
    void mouseMoveEvent(float x, float y, float xdelta, float ydelta);
    void mouseButtonEvent(int button, int action);
    void keyboardEvent(int key, int scancode, int action, int mods);
    void processKeyboardInput(float dt);
};




