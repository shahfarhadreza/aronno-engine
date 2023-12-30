#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "light.h"
#include "mesh.h"

World* World::sWorld = nullptr;

SceneEntity::SceneEntity(const std::string& name, Entity_T id)
    : mId(id), mName(name), mParent(nullptr) {

}

SceneEntity::SceneEntity(const std::string& name, Entity_T id, SceneEntity* parent)
    : mId(id), mName(name), mParent(parent) {
    if (parent) {
        parent->addChild(this);
    }
}

SceneEntity::~SceneEntity() {

}

void SceneEntity::addChild(SceneEntity* child) {
    mChildren.push_back(child);
}

void SceneEntity::update(float dt) {

}

World::World()
    : mViewTarget(nullptr), mSunLight(nullptr) {
    if (World::sWorld != nullptr) {
        assert(0);
    }
    World::sWorld = this;
}

World::~World() {
    for(auto it = mEntityList.begin(); it!= mEntityList.end();++it) {
        delete it->second;
    }
    mEntityList.clear();
    for(auto it = mPointLightComponents.begin(); it!= mPointLightComponents.end();++it) {
        delete it->second;
    }
    if (mSunLight)
        delete mSunLight;
}

SceneEntity* World::createEntity(const std::string& name) {
    Entity_T newId = mEntityList.size();
    SceneEntity* newEntity = new SceneEntity(name, newId);
    mEntityList[newId] = newEntity;
    return newEntity;
}

SceneEntity* World::createEntity(const std::string& name, SceneEntity* parent) {
    Entity_T newId = mEntityList.size();
    SceneEntity* newEntity = new SceneEntity(name, newId, parent);
    mEntityList[newId] = newEntity;
    return newEntity;
}

CameraEntity* World::createCamera(const std::string& name) {
    Entity_T newId = mEntityList.size();
    CameraEntity* newEntity = new CameraEntity(name, newId, {0, 0, 0});
    mEntityList[newId] = newEntity;
    return newEntity;
}

MeshComponent& World::addMeshComponent(SceneEntity* entity, Mesh* mesh) {
    Entity_T entityID = entity->getId();
    MeshComponent mc;
    mc.mMesh = mesh;
    mMeshComponents[entityID] = mc;
    return mMeshComponents[entityID];
}

MeshComponent& World::getMeshComponent(SceneEntity* entity) {
    return mMeshComponents[entity->getId()];
}

TransformComponent& World::addTransformComponent(SceneEntity* entity, const glm::vec3& pos, const glm::quat& qt, const glm::vec3& scale) {
    Entity_T entityID = entity->getId();
    TransformComponent tc;
    tc.Position = pos;
    tc.Orientation = qt;
    tc.Scale = scale;
    tc.Transform = glm::mat4(1.0f);
    tc.Transform = glm::translate(tc.Transform, tc.Position);
    tc.Transform = tc.Transform * glm::toMat4(tc.Orientation);
    tc.Transform = glm::scale(tc.Transform, tc.Scale);
    mTransformComponents[entityID] = tc;
    return mTransformComponents[entityID];
}

TransformComponent& World::getTransformComponent(SceneEntity* entity) {
    return mTransformComponents[entity->getId()];
}

bool World::hasTransformComponent(SceneEntity* entity) {
    return mTransformComponents.find(entity->getId()) != mTransformComponents.end();
}

InteractComponent& World::addInteractComponent(SceneEntity* entity, float distance) {
    Entity_T entityID = entity->getId();
    InteractComponent tc;
    tc.Distance = distance;
    mInteractComponents[entityID] = tc;
    return mInteractComponents[entityID];
}

InteractComponent& World::getInteractComponent(SceneEntity* entity) {
    return mInteractComponents[entity->getId()];
}

bool World::hasInteractComponent(SceneEntity* entity) {
    return mInteractComponents.find(entity->getId()) != mInteractComponents.end();
}

BillboardComponent& World::addBillboardComponent(SceneEntity* entity, Texture* tex) {
    Entity_T entityID = entity->getId();
    BillboardComponent tc;
    tc.Image = tex;
    mBillboardComponents[entityID] = tc;
    return mBillboardComponents[entityID];
}

BillboardComponent& World::getBillboardComponent(SceneEntity* entity) {
    return mBillboardComponents[entity->getId()];
}

bool World::hasBillboardComponent(SceneEntity* entity) {
    return mBillboardComponents.find(entity->getId()) != mBillboardComponents.end();
}

AnimationComponent& World::addAnimationComponent(SceneEntity* entity, float length) {
    Entity_T entityID = entity->getId();
    AnimationComponent tc;
    tc.Length = length;
    mAnimationComponents[entityID] = tc;
    return mAnimationComponents[entityID];
}

AnimationComponent& World::getAnimationComponent(SceneEntity* entity) {
    return mAnimationComponents[entity->getId()];
}

bool World::hasAnimationComponent(SceneEntity* entity) {
    return mAnimationComponents.find(entity->getId()) != mAnimationComponents.end();
}

PointLight* World::addPointLightComponent(SceneEntity* entity, const glm::vec3& pos, bool castShadow) {
    Entity_T entityID = entity->getId();
    PointLight* light = new PointLight(pos, castShadow);
    mPointLightComponents[entityID] = light;
    return mPointLightComponents[entityID];
}

PointLight* World::getPointLightComponent(SceneEntity* entity) {
    return mPointLightComponents[entity->getId()];
}

bool World::hasPointLightComponent(SceneEntity* entity) {
    return mPointLightComponents.find(entity->getId()) != mPointLightComponents.end();
}

DirectionalLight* World::createSunLight(bool castShadow) {
    if (!mSunLight)
        mSunLight = new DirectionalLight(castShadow);
    return mSunLight;
}

float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

void World::update(float dt) {

    // update all the root entities (the ones with no parent entity)
    for(auto itEnt = mEntityList.begin(); itEnt != mEntityList.end();++itEnt) {
        SceneEntity* entity = itEnt->second;
        entity->update(dt);
    }

    for(auto itEnt = mAnimationComponents.begin(); itEnt != mAnimationComponents.end();++itEnt) {
        SceneEntity* entity = mEntityList[itEnt->first];
        AnimationComponent& anim = itEnt->second;

        if (anim.Play) {

            if (anim.Reverse) {
                anim.Time = anim.Time - anim.Speed * dt;
            } else {
                anim.Time = anim.Time + anim.Speed * dt;
            }

            if (anim.Loop) {
                // Wrap
                anim.Time = fmod(anim.Time, anim.Length);
                if(anim.Time < 0) {
                    anim.Time += anim.Length;
                }
            } else {
                // Clamp
                if (anim.Time < 0) {
                    anim.Time = 0;
                    anim.Play = false;
                    anim.Reverse = false;
                }
                else if (anim.Time > anim.Length) {
                    anim.Time = anim.Length;
                    anim.Play = false;
                    anim.Reverse = false;
                }
            }

            float animationTime = anim.Time;

            int p0Index = anim.GetKeyFrameIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = getScaleFactor(anim.KeyFrames[p0Index].TimeStamp,
                anim.KeyFrames[p1Index].TimeStamp, animationTime);
            glm::quat finalRotation = glm::slerp(anim.KeyFrames[p0Index].Rotation,
                anim.KeyFrames[p1Index].Rotation, scaleFactor);
            finalRotation = glm::normalize(finalRotation);

            TransformComponent& trans = getTransformComponent(entity);

            trans.Orientation = finalRotation;
        }
    }

    // update all the bones
    for(auto itEnt = mMeshComponents.begin(); itEnt != mMeshComponents.end();++itEnt) {
        const MeshComponent& comp = itEnt->second;
        Mesh* mesh = comp.mMesh;

        SkeletonMesh* skeMesh = mesh->isSkeletonMesh();
        if (skeMesh) {
            Skeleton* skeleton = skeMesh->getSkeleton();
            skeleton->update(dt);
        }
    }

    for(auto itEnt = mTransformComponents.begin(); itEnt != mTransformComponents.end();++itEnt) {
        TransformComponent& tc = itEnt->second;

        tc.Transform = glm::mat4(1.0f);
        tc.Transform = glm::translate(tc.Transform, tc.Position);
        tc.Transform = tc.Transform * glm::toMat4(tc.Orientation);
        tc.Transform = glm::scale(tc.Transform, tc.Scale);
    }

    // update all the root entities (the ones with no parent entity)
    for(auto itEnt = mEntityList.begin(); itEnt != mEntityList.end();++itEnt) {
        SceneEntity* entity = itEnt->second;
        // we skip ones with parent (cause it will get updated in the recursive func when finding children)
        if (entity->isRootEntity()) {
            _updateEntityWorldTransform(entity, nullptr);
        }
    }
/*
    // update world space bounding boxes for all the meshes
    for(auto itEnt = mMeshComponents.begin(); itEnt != mMeshComponents.end();++itEnt) {
        const MeshComponent& comp = itEnt->second;
        Mesh* mesh = comp.mMesh;
        Entity_T entityID = itEnt->first;

        glm::mat4 bbTransform;
        char hasWorldTransform = 0;

        auto itTrans = mWorldTransforms.find(entityID);
        if (itTrans != mWorldTransforms.end()) {
            bbTransform = itTrans->second;
            hasWorldTransform = 1;
        }

        const auto& sml = mesh->getSubMeshList();

        for (auto it = sml.begin(); it != sml.end();++it) {
            SubMesh* sm = (*it);

            AABB localBB = sm->getLocalBoundingBox();
            if (hasWorldTransform == 1) {
                localBB.transform(bbTransform);
            }
            sm->setWorldBoundingBox(localBB);
        }
    }
*/
}

void World::_updateEntityWorldTransform(SceneEntity* entity, SceneEntity* parent) {

    // if an entity has a local transform component,
    // then we must have to compute a world transform for it
    // by visiting through its hiarochy
    Entity_T id = entity->getId();

    glm::mat4 localTransform;

    auto itTrans = mTransformComponents.find(id);
    if (itTrans != mTransformComponents.end()) {
        localTransform = itTrans->second.Transform;;
    } else {
        localTransform = glm::mat4(1.0);
    }

    if (parent != nullptr) {
        Entity_T parentId = parent->getId();
        auto itParentTrans = mTransformComponents.find(parentId);
        if (itParentTrans != mTransformComponents.end()) {

            auto itParentWorldTrans = mWorldTransforms.find(parentId);
            assert(itParentWorldTrans != mWorldTransforms.end());

            mWorldTransforms[id] = mWorldTransforms[parentId] * localTransform;

        } else {
            mWorldTransforms[id] = localTransform;
        }
    } else {
        mWorldTransforms[id] = localTransform;
    }

    // visit and update the children
    const auto& children = entity->getChildren();
    for(auto itEnt = children.begin(); itEnt != children.end();++itEnt) {
        SceneEntity* child = *itEnt;
        _updateEntityWorldTransform(child, entity);
    }
}








