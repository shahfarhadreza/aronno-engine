#pragma once

class SceneEntity;
class Mesh;
class Light;
class DirectionalLight;
class PointLight;

class SceneEntity
{
protected:
    Entity_T mId;
    std::string mName;
    SceneEntity* mParent;
    std::vector<SceneEntity*> mChildren;
public:
    SceneEntity(const std::string& name, Entity_T id);
    SceneEntity(const std::string& name, Entity_T id, SceneEntity* parent);
    virtual ~SceneEntity();

    Entity_T getId() const { return mId; }

    bool isRootEntity() const { return mParent == nullptr; }

    const std::vector<SceneEntity*>& getChildren() const {
        return mChildren;
    }

    const std::string& getName() const { return mName; }
    void setName(const std::string& name) { mName = name; }

    virtual void update(float dt);
protected:
    void addChild(SceneEntity* child);
};

struct TransformComponent
{
    glm::vec3 Position;
    glm::quat Orientation;
    glm::vec3 Scale;
    glm::mat4 Transform;
};

struct MeshComponent
{
    Mesh* mMesh;
};

struct BillboardComponent
{
    Texture* Image;
    float Opacity;
};

class IInteractionHandler
{
public:
    IInteractionHandler() { }
    virtual ~IInteractionHandler() { }

    virtual void interaction() = 0;
};

struct InteractComponent
{
    float Distance;
    std::string Text;
    Texture* Icon;
    IInteractionHandler* Handler;

    InteractComponent() {
        Handler = nullptr;
    }
};

struct KeyFrame {
    glm::quat Rotation;
    float TimeStamp;
};

struct AnimationComponent
{
    float Length;
    float Time;
    float Speed;
    bool Loop;
    bool Reverse;
    bool Play;
    std::vector<KeyFrame> KeyFrames;

    void createKeyFrame(const glm::quat& rot, float timeStamp) {
        KeyFrame frame;
        frame.Rotation = rot;
        frame.TimeStamp = timeStamp;
        KeyFrames.push_back(frame);
    }

    int GetKeyFrameIndex(float animationTime)
    {
        for (size_t index = 0; index < KeyFrames.size() - 1; ++index)
        {
            if (animationTime < KeyFrames[index + 1].TimeStamp)
                return index;
        }
        return 0;
    }
};

class EntitySystem
{
public:
    EntitySystem() { }
    virtual ~EntitySystem() { }
};

class World
{
private:
    static World* sWorld;
public:
    std::unordered_map<Entity_T, glm::mat4> mWorldTransforms;
public:
    std::unordered_map<Entity_T, SceneEntity*> mEntityList;
    std::unordered_map<Entity_T, TransformComponent> mTransformComponents;
    std::unordered_map<Entity_T, MeshComponent> mMeshComponents;
    std::unordered_map<Entity_T, InteractComponent> mInteractComponents;
    std::unordered_map<Entity_T, BillboardComponent> mBillboardComponents;
    std::unordered_map<Entity_T, PointLight*> mPointLightComponents;
    std::unordered_map<Entity_T, AnimationComponent> mAnimationComponents;
    CameraEntity* mViewTarget;
    DirectionalLight* mSunLight;
public:
    World();
    virtual ~World();

    static World* get() { return sWorld; }

    SceneEntity* createEntity(const std::string& name);
    SceneEntity* createEntity(const std::string& name, SceneEntity* parent);
    CameraEntity* createCamera(const std::string& name);

    void setViewTarget(CameraEntity* entity) {
        mViewTarget = entity;
    }
    CameraEntity* getViewTarget() { return mViewTarget; }

    MeshComponent& addMeshComponent(SceneEntity* entity, Mesh* mesh);
    MeshComponent& getMeshComponent(SceneEntity* entity);

    TransformComponent& addTransformComponent(SceneEntity* entity, const glm::vec3& pos, const glm::quat& qt, const glm::vec3& scale);
    TransformComponent& getTransformComponent(SceneEntity* entity);
    bool hasTransformComponent(SceneEntity* entity);

    InteractComponent& addInteractComponent(SceneEntity* entity, float distance);
    InteractComponent& getInteractComponent(SceneEntity* entity);
    bool hasInteractComponent(SceneEntity* entity);

    BillboardComponent& addBillboardComponent(SceneEntity* entity, Texture* tex);
    BillboardComponent& getBillboardComponent(SceneEntity* entity);
    bool hasBillboardComponent(SceneEntity* entity);

    PointLight* addPointLightComponent(SceneEntity* entity, const glm::vec3& pos, bool castShadow);
    PointLight* getPointLightComponent(SceneEntity* entity);
    bool hasPointLightComponent(SceneEntity* entity);

    AnimationComponent& addAnimationComponent(SceneEntity* entity, float length);
    AnimationComponent& getAnimationComponent(SceneEntity* entity);
    bool hasAnimationComponent(SceneEntity* entity);

    DirectionalLight* createSunLight(bool castShadow);
    virtual void update(float dt);
protected:
    void _updateEntityWorldTransform(SceneEntity* entity, SceneEntity* parent);
};




