#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"

#include "game.h"

void DemonBase::triggerDamage() {
    if (mHealth > 0) {
        mHealth -= 10;
    } else {
        markForRemove();
    }
}

void DemonBase::update(float dt) {
    btTransform trans = getGhostObject()->getWorldTransform();

    glm::vec3 demonPos(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));

    //glm::vec3 demonPos(1, 120, 1);

    // update matrix
    mTransform = glm::mat4(1.0f);
    mTransform = glm::translate(mTransform, demonPos );

}

