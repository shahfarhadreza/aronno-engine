#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "game.h"

Player::Player(btDiscreteDynamicsWorld* dynamicsWorld, const glm::vec3& origin, SceneEntity* entity, CameraEntity* camera)
    : Character(dynamicsWorld, origin), mPlayerEntity(entity), mCamera(camera), mAnimationTime(0) {

    mShakeAnimationTime = 0;

    mHandTransform = &World::get()->getTransformComponent(mPlayerEntity);

    SkeletonMesh* mesh = World::get()->getMeshComponent(mPlayerEntity).mMesh->isSkeletonMesh();
    mSkeHands = mesh->getSkeleton();

    //mAnimIdle = mSkeHands->getAnimationState("Armature|FN_Walk");
    //mAnimIdle->setSpeed(10);
    //mSkeHands->setAnimationState(mAnimIdle);

    mAnimShot = mSkeHands->getAnimationState("Armature|FN_Shot");
    mAnimShot->setSpeed(1600);

    mAnimWalk = mSkeHands->getAnimationState("Armature|FN_Walk");
    mAnimWalk->setSpeed(500);
    mAnimWalk->setLoop(true);

    mSkeHands->setAnimationState(mAnimWalk);

    mAnimReload = mSkeHands->getAnimationState("Armature|FN_ReloadFull");
    mAnimReload->setSpeed(1000);

    // Setup muzzle flash
    SceneEntity* muzzleEntity = World::get()->createEntity("muzzle", mPlayerEntity);
    mMuzzle = &World::get()->addBillboardComponent(muzzleEntity, Game::get()->mMuzzleTexture);
    mMuzzle->Opacity = 0;

    glm::quat qt2 = glm::quat(glm::vec3(glm::radians(0.0f), 0, glm::radians(20.0f)));
    mMuzzleTransform = &World::get()->addTransformComponent(muzzleEntity, {-0.14, -0.03, 0.95}, qt2, {1, 1, 1});

    mMuzzleScale = 0.1f;
    mMuzzleTransform->Scale = {1.8 * 0.3f * mMuzzleScale, 1 * 0.3f * mMuzzleScale, 1 * 0.3f * mMuzzleScale};

    // Muzzle flash light
    mMuzzleLight = World::get()->addPointLightComponent(muzzleEntity, {0, 0, 0}, false);
    mMuzzleLight->setName("Gun Muzzle Flash");
    mMuzzleLight->setColor({0.8, 0.6, 0.2});
    mMuzzleLight->setEnabled(false);
}

void Player::triggerShoot() {
    if (mClipAmmo > 0) {
        mClipAmmo--;
        mShakeAnimationTime = 15;
        mMuzzle->Opacity = 1;
        mMuzzleScale = 0.1f;

        mMuzzleLight->setIntensity(5);
        mMuzzleLight->setEnabled(true);
        mAnimShot->setTime(0); // Reset on each click
        mSkeHands->setAnimationState(mAnimShot);
    } else {
        mMuzzle->Opacity = 0;
        mSkeHands->setAnimationState(mAnimReload);
    }
}

void Player::update(float dt) {
    //btTransform trans = getGhostObject()->getWorldTransform();

    if (mSkeHands->getCurrentAnimationState() == mAnimShot) {
        if (mAnimShot->hasEnded()) {
            mAnimShot->setTime(0);
            mSkeHands->setAnimationState(mAnimWalk);
            mMuzzle->Opacity = 0;
            mMuzzleScale = 0.1f;
            mMuzzleLight->setEnabled(false);
        } else {

            float light = mMuzzleLight->getIntensity();

            light -= dt * 40;

            mMuzzle->Opacity -= dt * 10;
            mMuzzleScale += dt * 25;
            if ( mMuzzle->Opacity < 0) {
                mMuzzle->Opacity = 0;
            }
            if (mMuzzleScale > 1) {
                mMuzzleScale = 1;
            }

            if (light <= 0) {
                light = 0;
            }

            mMuzzleLight->setIntensity(light);
        }

        mMuzzleTransform->Scale = {1.8 * 0.3f * mMuzzleScale, 1 * 0.3f * mMuzzleScale, 1 * 0.3f * mMuzzleScale};
    }

    if (mSkeHands->getCurrentAnimationState() == mAnimReload) {
        if (mAnimReload->hasEnded()) {
            mClipAmmo = mMaxClipAmmo;
            mAmmo -= mClipAmmo;
            mAnimReload->setTime(0);
            mSkeHands->setAnimationState(mAnimWalk);
        }
    }

    btVector3 vel = getController()->getLinearVelocity();
    btVector3 velXZ(vel.getX(), 0.0f, vel.getZ());

    float velocity = velXZ.length() * 5;

    if (velocity < 0.1) {
        velocity = 0.25;
    }

    float offset_factor = sin(mAnimationTime);

    // add some shake effect
    mHandTransform->Position = {2, -7 + (offset_factor * 0.1), -5};
    mCamera->setShakeOffset(mShakeAnimationTime * 0.05);

    // do bobbing math
    mAnimationTime += dt * 15 * velocity;

    if (mAnimationTime >= 6.2) {
        mAnimationTime = 0;
    }

    if (mShakeAnimationTime > 0) {
        mShakeAnimationTime -= dt * 60;
    }
}

