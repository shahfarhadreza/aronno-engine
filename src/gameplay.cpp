#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"

#include "game.h"


BulletProjectile::BulletProjectile(const glm::vec3& origin, const glm::vec3& dir)
    : mOrigin(origin), mDirection(dir) {

    mPosition = mOrigin;
    mSpeed = 600.0f;
    mMaxDistance = 300;
/*
    for(auto& v : projectileVertices) {
        mLocalBoundingBox.extend(v.position);
    }
*/
}

void PopupText::update(float dt) {
    if (mTime >= mDuration) {
        markForRemove();
    } else {
        mTime += dt * 100.0f;
    }
}

InteractionManager* InteractionManager::sStatic = nullptr;

InteractionManager::InteractionManager() {
    if (InteractionManager::sStatic != nullptr) {
        assert(0);
    }
    InteractionManager::sStatic = this;
    mLastObject = nullptr;
    mActiveObject = nullptr;
    mComponent = nullptr;
    mAnimationTime = 1.0f;
}

void InteractionManager::update(float dt) {

    if (mActiveObject != mLastObject) {
        mLastObject = mActiveObject;
        mAnimationTime = 1;
    }

    if (mActiveObject != nullptr) {
        mAnimationTime -= dt * 1.1;
        if (mAnimationTime < 0) {
            mAnimationTime = 1;
        }
    }
}

CollisionObject::CollisionObject() {
    mTransform.setIdentity();
}

CollisionObject::~CollisionObject() {
    delete mRigidBody;
    delete mMotionState;
    delete mColShape;
}

CollisionManager::CollisionManager() {
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    dynamicsWorld->setGravity(btVector3(0.0f, -9.8f, 0.0f));
}

CollisionManager::~CollisionManager() {
    for(auto& o : mCollisionObjects) {
        delete o.second;
    }
    //delete dynamics world
    delete dynamicsWorld;
    //delete solver
    delete solver;
    //delete broadphase
    delete overlappingPairCache;
    //delete dispatcher
    delete dispatcher;
    delete collisionConfiguration;
}

void CollisionManager::registerStaticEntity(SceneEntity* entity) {
    // create a mesh shape for the level
    btVector3 localInertia(0, 0, 0);

    CollisionObject* cobj = new CollisionObject();

    auto levelMeshBullet = World::get()->getMeshComponent(entity).mMesh->getCollisionMesh();

    cobj->mColShape = new btBvhTriangleMeshShape(levelMeshBullet, true);

    if (World::get()->hasTransformComponent(entity)) {
        TransformComponent& trans = World::get()->getTransformComponent(entity);

        const glm::vec3& pos = trans.Position;
        cobj->mTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    } else {
        cobj->mTransform.setOrigin(btVector3(0, 0, 0));
    }

    btScalar levelMass(0);

    cobj->mColShape->calculateLocalInertia(levelMass, localInertia);

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    cobj->mMotionState = new btDefaultMotionState(cobj->mTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo1(levelMass, cobj->mMotionState, cobj->mColShape, localInertia);
    cobj->mRigidBody = new btRigidBody(rbInfo1);

    dynamicsWorld->addRigidBody(cobj->mRigidBody);

    mCollisionObjects[entity] = cobj;
}

void CollisionManager::update(float dt) {
    // update bullet physic
    dynamicsWorld->stepSimulation(dt * 10, 4, 1.f / 60.f);

    World* world = World::get();
    CameraEntity* cam = world->getViewTarget();

    InteractionManager::get()->mActiveObject = nullptr;
    InteractionManager::get()->mComponent = nullptr;

    for(auto& o : mCollisionObjects) {
        SceneEntity* entity = o.first;
        CollisionObject* cobj = o.second;

        if (world->hasTransformComponent(entity)) {
            TransformComponent& trans = world->getTransformComponent(entity);

            const glm::vec3& pos = trans.Position;
            const glm::quat& qt = trans.Orientation;
            const glm::vec3& scale = trans.Scale;

            btTransform btTrans = cobj->mTransform;
            btTrans.setOrigin(btVector3(pos.x, pos.y, pos.z));
            btTrans.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));

            cobj->mRigidBody->setWorldTransform(btTrans);
            cobj->mTransform = btTrans;

/*
            DO WE NEED THIS NOW??

            cobj->mColShape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
            dynamicsWorld->updateSingleAabb(cobj->mRigidBody);
*/
        }

        if (world->hasInteractComponent(entity)) {
            InteractComponent& interact = world->getInteractComponent(entity);

            const glm::vec3& camPos = cam->mTransform->Position;
            const glm::vec3& camDir = cam->getDirection();

            btVector3 from(camPos.x, camPos.y, camPos.z);

            float rayLength = interact.Distance;

            btVector3 to = from + btVector3(rayLength, rayLength, rayLength) * btVector3(camDir.x, camDir.y, camDir.z);

            btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
            closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

            btTransform transFrom;
            transFrom.setIdentity();
            transFrom.setOrigin(from);

            btTransform transTo;
            transTo.setIdentity();
            transTo.setOrigin(to);

			dynamicsWorld->rayTestSingle(transFrom, transTo, cobj->mRigidBody, cobj->mColShape, cobj->mTransform, closestResults);

			if (closestResults.hasHit()) {
                InteractionManager::get()->mActiveObject = entity;
                InteractionManager::get()->mComponent = &interact;
                break;
            }
        }
    }
}

glm::quat& FromToQuat(glm::quat& qt, const glm::vec3& from, const glm::vec3& to) {
    const glm::vec3& v1 = to;
    const glm::vec3& v2 = from;
    glm::vec3 a = glm::cross(v1, v2);
    qt.x = a.x;
    qt.y = a.y;
    qt.z = a.z;
    qt.w = sqrtf((glm::length2(v1)) * (glm::length2(v2) )) + glm::dot(v1, v2);
    return qt;
}

Decal::Decal(const glm::vec3& origin, const glm::vec3& dir)
    : mOrigin(origin), mDirection(dir) {

    mDirection = glm::normalize(mDirection);

    // Update the transformation matrix as well
    mTransform = glm::mat4(1.0f);
    mTransform = glm::translate(mTransform, mOrigin + (mDirection * 0.025f));

    glm::quat qt;
    FromToQuat(qt, {0, 0, 1}, mDirection);

    mTransform = mTransform * glm::toMat4(glm::normalize(qt));

    mTransform = glm::scale(mTransform, glm::vec3(0.2, 0.2, 0.2));
}

void Decal::update(float dt) {

}

void GamePlayState::start(GameState* prev) {

}

void GamePlayState::update(float dt) {

}

void GamePlayState::render() {

}



