#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"

#include "game.h"

const float STEP_HEIGHT = 0.7f;
const float JUMP_HEIGHT = 1.5f;
const float FALL_SPEED = 55.0f;
const float JUMP_SPEED = 30.0f;
const float MAX_SLOPE = 45.0f;
const float DEFAULT_DAMPING = 0.1f;
const btVector3 KINEMATIC_GRAVITY(0.0f, -9.8f, 0.0f);

Character::Character(btDiscreteDynamicsWorld* dynamicsWorld, const glm::vec3& origin)
    : mDynamicsWorld(dynamicsWorld), mHealth(100), mAmmo(40), mClipAmmo(10), mMaxClipAmmo(10) {
    // create a capsule rigid body for the player

    mCollisionShape = new btCapsuleShapeZ(btScalar(16), btScalar(12));

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(origin.x, origin.y, origin.z));

    mGhostObject = new btPairCachingGhostObject();
    mGhostObject->setCollisionShape(mCollisionShape);
	mGhostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	mGhostObject->setWorldTransform(startTransform);

	mController = new btKinematicCharacterController(mGhostObject, mCollisionShape, STEP_HEIGHT);

    mController->setLinearDamping(DEFAULT_DAMPING);
    mController->setAngularDamping(DEFAULT_DAMPING);
    mController->setStepHeight(STEP_HEIGHT);
    mController->setMaxJumpHeight(JUMP_HEIGHT);
    mController->setMaxSlope(M_DEGTORAD * MAX_SLOPE);
    mController->setJumpSpeed(JUMP_SPEED);
    mController->setFallSpeed(FALL_SPEED);
    mController->setGravity(KINEMATIC_GRAVITY);

    dynamicsWorld->addCollisionObject(mGhostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
    dynamicsWorld->addAction(mController);

    //dynamicsWorld->getCollisionWorld()->rayTest()
}

Character::~Character() {
    if (mController) {
        mDynamicsWorld->removeAction(mController);
        delete mController;
    }
    if (mGhostObject) {
        mDynamicsWorld->removeCollisionObject(mGhostObject);
        delete mGhostObject;
    }
    if (mCollisionShape) {
        delete mCollisionShape;
    }
}

void Character::setPosition(const glm::vec3& origin) {
    if (mGhostObject) {
        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(origin.x, origin.y, origin.z));

        mGhostObject->setWorldTransform(startTransform);
    }
}





