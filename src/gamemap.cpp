#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"

#include "game.h"

#include "textparser.h"

bool Game::loadMap(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }
    std::string curline;
    while (std::getline(file, curline)) {
        if (obj_firstToken(curline) == "lights") {
            int lightCout = std::stoi(obj_tail(curline));
            printf("total lights %d\n", lightCout);
        }
/*
        if (obj_firstToken(curline) == "light") {
            std::vector<std::string> props;
            obj_split(obj_tail(curline), props, ",");
            std::string name = props[0];
            bool castShadows = trim(props[1]) == "true" ? true : false;
            glm::vec3 pos = {std::stof(props[2]), std::stof(props[3]), std::stof(props[4])};

            SceneEntity* lightEntity = mWorld->createEntity(name);
            PointLight* light = mWorld->addPointLightComponent(lightEntity, pos, castShadows);
            light->setName(name);
        }
*/
        if (obj_firstToken(curline) == "player") {
            std::vector<std::string> props;
            obj_split(obj_tail(curline), props, ",");
            glm::vec3 pos = {std::stof(props[0]), std::stof(props[1]), std::stof(props[2])};
            mPlayerCharacter->setPosition(pos);
        }

        if (obj_firstToken(curline) == "enemy") {
            std::vector<std::string> props;
            obj_split(obj_tail(curline), props, ",");
            glm::vec3 pos = {std::stof(props[0]), std::stof(props[1]), std::stof(props[2])};
            // create demons!
            DemonBase* demon;
            demon = new DemonBase(mDemonMesh, mCollisionMgr->getDynamicsWorld(), pos);
            mEnemyCharacterList.push_back(demon);
        }

        if (obj_firstToken(curline) == "sun") {
            std::vector<std::string> props;
            obj_split(obj_tail(curline), props, ",");
            //glm::vec3 pos = {std::stof(props[0]), std::stof(props[1]), std::stof(props[2])};
            //mSunLight->setPosition(pos);
        }

        if (obj_firstToken(curline) == "door") {
            std::vector<std::string> props;
            obj_split(obj_tail(curline), props, ",");
            glm::vec3 pos = {std::stof(props[0]), std::stof(props[1]), std::stof(props[2])};

            SceneEntity* doorEntity = mWorld->createEntity("door");
            mWorld->addMeshComponent(doorEntity , mDoorMesh);

            glm::quat qt = glm::quat(glm::vec3(0, 0, 0));

            mWorld->addTransformComponent(doorEntity, pos, qt, {1, 1, 1});
            InteractComponent& interact = mWorld->addInteractComponent(doorEntity, 50);
            interact.Text = "Press 'E' To Interact";
            interact.Icon = mDoorInteractTex;

            AnimationComponent& anim = mWorld->addAnimationComponent(doorEntity, 10);
            anim.Time = 0;
            anim.Speed = 10;
            anim.Loop = false;
            anim.Play = false;
            anim.Reverse = false;

            anim.createKeyFrame(glm::quat(glm::vec3(glm::radians(0.0f), glm::radians(0.0f), 0.0f)), 0);
            anim.createKeyFrame(glm::quat(glm::vec3(glm::radians(0.0f), glm::radians(90.0f), 0.0f)), 10);

            mCollisionMgr->registerStaticEntity(doorEntity);
        }

        if (obj_firstToken(curline) == "dumpster") {
            std::vector<std::string> props;
            obj_split(obj_tail(curline), props, ",");
            glm::vec3 pos = {std::stof(props[0]), std::stof(props[1]), std::stof(props[2])};

            SceneEntity* dumpsterEntity = mWorld->createEntity("dumpster");
            mWorld->addMeshComponent(dumpsterEntity, mDumpsterMesh);
            mWorld->addTransformComponent(dumpsterEntity, pos, glm::quat(glm::vec3(0, 0, 0)), {1, 1, 1});

            SceneEntity* dumpsterLidEntity = mWorld->createEntity("dumpster_lid");
            mWorld->addMeshComponent(dumpsterLidEntity , mDumpsterLidMesh);
            mWorld->addTransformComponent(dumpsterLidEntity, {pos.x, pos.y + 44, pos.z}, glm::quat(glm::vec3(0, 0, 0)), {1, 1, 1});

            InteractComponent& interact = mWorld->addInteractComponent(dumpsterLidEntity, 100);
            interact.Text = "Press 'E' To Interact";
            interact.Icon = mDumpsterInteractTex;

            AnimationComponent& anim = mWorld->addAnimationComponent(dumpsterLidEntity, 10);
            anim.Time = 0;
            anim.Speed = 10;
            anim.Loop = false;
            anim.Play = false;
            anim.Reverse = false;

            anim.createKeyFrame(glm::quat(glm::vec3(glm::radians(0.0f), glm::radians(0.0f), 0.0f)), 0);
            anim.createKeyFrame(glm::quat(glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(-50.0f))), 10);

            mCollisionMgr->registerStaticEntity(dumpsterEntity);
            mCollisionMgr->registerStaticEntity(dumpsterLidEntity);
        }
    }
    return true;
}



