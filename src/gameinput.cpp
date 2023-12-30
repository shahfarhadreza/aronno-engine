#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"

#include "game.h"

void Game::keyboardEvent(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        SceneEntity* iobj = mInteractionMgr.mActiveObject;
        if (iobj) {
            AnimationComponent& anim = mWorld->getAnimationComponent(iobj);
            if (anim.Time == 0) {
                anim.Time = 0;
                anim.Play = true;
                anim.Reverse = false;
            } else {
                anim.Play = true;
                anim.Reverse = true;
            }
        }
        if (iobj && mInteractionMgr.mComponent->Handler) {
            mInteractionMgr.mComponent->Handler->interaction();

            //TransformComponent& trans = mWorld->getTransformComponent(iobj);
            //trans.Orientation = glm::quat(glm::vec3(glm::radians(0.0f), glm::radians(-90.0f), 0.0f));
        }
    }
}

void Game::processKeyboardInput(float dt) {
    GLFWwindow* window = mWindow;
    //float cameraSpeed = 50.0f * dt;

    glm::vec3 camRot = mCamera->getDirection();
    glm::vec3 moveDir(0, 0, 0);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveDir += glm::vec3(1.0, 0.0, 1.0) * camRot;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveDir += glm::vec3(-1.0, 0.0, -1.0) * camRot;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm::vec3 v = glm::cross(mCamera->getUp(), camRot);
        moveDir += glm::vec3(1.0, 0.0, 1.0) * v;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm::vec3 v = glm::cross(mCamera->getUp(), camRot);
        moveDir += glm::vec3(-1.0, 0.0, -1.0) * v;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (mPlayerCharacter->getController()->canJump()) {
            mPlayerCharacter->getController()->jump();
        }
    }

    btVector3 curMoveDir(moveDir.x, moveDir.y, moveDir.z);
    mPlayerCharacter->getController()->setWalkDirection(curMoveDir * 0.25f);

/*
    glm::vec3 lightPos = mSunLight->getPosition();

    // for light movement
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        lightPos.z -= cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        lightPos.z += cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        lightPos.x += cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        lightPos.x -= cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        lightPos.y += cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        lightPos.y -= cameraSpeed;
    //printf("x %f, z %f\n", lightPos.x, lightPos.z);

    mSunLight->setPosition(lightPos);
*/
}

void Game::mouseMoveEvent(float x, float y, float xdelta, float ydelta) {
    mCamera->rotate(xdelta, ydelta);
}

void Game::mouseButtonEvent(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

        if (mPlayerCharacter->hasAmmo()) {
            mPlayerCharacter->triggerShoot();

            World* world = World::get();
            CameraEntity* cam = world->getViewTarget();

            glm::vec3 camPos = cam->mTransform->Position;
            glm::vec3 camDir = cam->getDirection();

            btVector3 from(camPos.x, camPos.y, camPos.z);

            float rayLength = 2000.0f;

            btVector3 to = from + btVector3(rayLength, rayLength, rayLength) * btVector3(camDir.x, camDir.y, camDir.z);

            btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
            closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

			mCollisionMgr->getDynamicsWorld()->rayTest(from, to, closestResults);

			//printf("to %f, %f, %f\n", to.getX(), to.getY(), to.getZ());

			if (closestResults.hasHit()) {
                btVector3 p = closestResults.m_hitPointWorld;
                btVector3 n = closestResults.m_hitNormalWorld;
                //printf("p %f, %f, %f\n", p.getX(), p.getY(), p.getZ());
                //printf("n %f, %f, %f\n", n.getX(), n.getY(), n.getZ());

                // Let's make the bullet projectile travel from gun point to hit point

                glm::vec3 hitPoint(p.getX(), p.getY(), p.getZ());
                glm::vec3 hitNormal(n.getX(), n.getY(), n.getZ());

                Decal newDecal(hitPoint, hitNormal);
                mDecals.push_back(newDecal);

                /*
                glm::vec3 vecSide = glm::cross(mCamera->getUp(), camDir);

                glm::vec3 origin = camPos;

                bool scifi = false;
                // adjust/offset to the gun point (so that bullets look like coming out of the gun)
                glm::vec3 offset;

                if (scifi) {
                    offset = {-8, -2, 28};
                } else {
                    offset = {-8, -2, 28};
                }

                origin = origin + glm::vec3(offset.x, offset.x, offset.x) * vecSide;
                origin = origin + glm::vec3(offset.y, offset.y, offset.y) * mCamera->getUp();
                origin = origin + glm::vec3(offset.z, offset.z, offset.z) * camDir;

                glm::vec3 bulletDir = hitPoint - origin;
                bulletDir = glm::normalize(bulletDir);

                BulletProjectile newBullet(origin, bulletDir);
                mBulletProjectiles.push_back(newBullet);
                */
			}
        }
    }
}

