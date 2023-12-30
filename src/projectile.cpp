#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"

#include "game.h"

void BulletProjectile::update(float dt) {

    // Simply travel through the given direction
    glm::vec3 accelaration = mDirection * mSpeed * dt;
    mPosition = mPosition + accelaration;

    // Update the transformation matrix as well
    mTransform = glm::mat4(1.0f);
    mTransform = glm::translate(mTransform, mPosition );

    AABB worldBoundingBox(mLocalBoundingBox);
    worldBoundingBox.transform(mTransform);

    mWorldBoundingBox = worldBoundingBox;
}

