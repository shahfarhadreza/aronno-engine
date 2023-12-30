#include "stdafx.h"
#include "engine.h"
#include "world.h"
#include "camera.h"

CameraEntity::CameraEntity(const std::string& name, Entity_T id, const glm::vec3& position)
    : SceneEntity(name, id), mDirection({0, 0, 0}),
    mWorldUp(0.0f, 1.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f),
    mRight(1, 0, 0), mYaw(0), mPitch(0), mPitchLimit(88.0f),
    mShakeOffset(0), mNear(1.0f), mFar(5000.0f), mFOV(60.0f) {
}

void CameraEntity::updateProjection(float width, float height) {
    mAspect = float(width) / float(height);
    mProjection = glm::perspective(glm::radians(mFOV), mAspect, mNear, mFar);
}

void CameraEntity::update(float dt) {
    glm::vec3 direction;
    float pitch = mPitch + mShakeOffset;
    direction.x = cos(glm::radians(mYaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(mYaw)) * cos(glm::radians(pitch));

    mDirection = glm::normalize(direction);

    mRight = glm::normalize(glm::cross(mDirection, mWorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    mUp    = glm::normalize(glm::cross(mRight, mDirection));

    mTransform->Orientation = glm::normalize(glm::quatLookAt(mDirection, mUp));
    //mView = glm::lookAt(mPosition, mPosition + mDirection, mUp);
}

