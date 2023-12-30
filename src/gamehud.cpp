#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"

#include "game.h"

void prepareText(const std::string& text, const glm::vec2& pos, float size, std::vector<Vertex>& vertices, float offx = .6f);

void Game::renderHUD() {
    mRend->bindResource(hudProgram);
    mRend->bindQuadBuffer(mHUDQuad);

    mPerFrameData.proj = glm::ortho(0.0f, float(SCR_WIDTH), float(SCR_HEIGHT), 0.0f, -2.1f, 10.0f);
    mCBPerFrame->updateData(&mPerFrameData);

    glm::mat4 matHud;

    mPerObjectData.opacity = 1;

    // Crosshair
    matHud = glm::mat4(1.0f);
    matHud = glm::translate(matHud, glm::vec3( float(SCR_WIDTH) / 2, float(SCR_HEIGHT) / 2, 0) );
    matHud = glm::scale(matHud, glm::vec3( 27.0f, 27, 0) );
    mPerObjectData.world = matHud;
    mCBPerObject->updateData(&mPerObjectData);

    mRend->bindGPUTexture(mCrosshairTexture, 3);
    mRend->draw(mHUDQuad->getIndexCount());

    float healthHudX = 100;
    float bulletHudX = float(SCR_WIDTH) - 260;
    float bulletHudY = float(SCR_HEIGHT) - 100;

    // Health
    matHud = glm::mat4(1.0f);
    matHud = glm::translate(matHud, glm::vec3( healthHudX, float(SCR_HEIGHT) - 100, 0) );
    matHud = glm::scale(matHud, glm::vec3( 80.0f, 90, 0) );
    mPerObjectData.world = matHud;
    mCBPerObject->updateData(&mPerObjectData);

    mRend->bindGPUTexture(mHudHealthTexture, 3);
    mRend->draw(mHUDQuad->getIndexCount());

    // Ammo
    matHud = glm::mat4(1.0f);
    matHud = glm::translate(matHud, glm::vec3( bulletHudX, bulletHudY, 0) );
    matHud = glm::scale(matHud, glm::vec3( 90.0f, 75, 0) );
    mPerObjectData.world = matHud;
    mCBPerObject->updateData(&mPerObjectData);

    mRend->bindGPUTexture(mHudBulletTexture, 3);
    mRend->draw(mHUDQuad->getIndexCount());

    // Interaction Icon
    if (mInteractionMgr.mActiveObject != nullptr) {

        const InteractComponent& interact = mWorld->getInteractComponent(mInteractionMgr.mActiveObject);

        if (interact.Icon != nullptr) {
            matHud = glm::mat4(1.0f);
            matHud = glm::translate(matHud, glm::vec3( float(SCR_WIDTH) / 2, float(SCR_HEIGHT) - 90, 0) );
            matHud = glm::scale(matHud, glm::vec3( 30, 30, 0) );
            mPerObjectData.world = matHud;
            mPerObjectData.opacity = mInteractionMgr.mAnimationTime;
            mCBPerObject->updateData(&mPerObjectData);

            mRend->bindGPUTexture(interact.Icon->getGPUResource(), 3);
            mRend->draw(mHUDQuad->getIndexCount());
        }
    }

    mPerObjectData.opacity = 1;

    // Draw Texts
    matHud = glm::mat4(1.0f);
    mPerObjectData.world = matHud;
    mCBPerObject->updateData(&mPerObjectData);

    mRend->bindGPUTexture(mHudFontTexture, 3);

    // create and update
    std::vector<Vertex> vertices;

    // Draw some debug texts first
    prepareText(debugText, {10, 10}, 20, vertices);

    mRend->bindResource(mHUDMsgTextVB);
    mHUDMsgTextVB->updateData(&vertices[0], vertices.size());
    mRend->drawNonIndexed(vertices.size());

    vertices.clear();

    char text[128];
/*
    sprintf(text, "%d", mPlayerCharacter->getHealth());
    prepareText(text, {healthHudX + 40, float(SCR_HEIGHT) - 85}, 30, vertices);
    mRend->bindResource(mHUDTextHealthVB);
    mHUDTextHealthVB->updateData(&vertices[0], vertices.size());
    mRend->drawNonIndexed(vertices.size());
    vertices.clear();
*/
    // Interaction Text
    if (mInteractionMgr.mActiveObject != nullptr) {

        const InteractComponent& interact = mWorld->getInteractComponent(mInteractionMgr.mActiveObject);

        if (!interact.Text.empty()) {

            //matHud = glm::mat4(1.0f);
            //matHud = glm::scale(matHud, glm::vec3( 1, 1, 1) );
            //mPerObjectData.world = matHud;
            //mCBPerObject->updateData(&mPerObjectData);

            prepareText(interact.Text, {(float(SCR_WIDTH) / 2) - 120, float(SCR_HEIGHT) - 150}, 20, vertices);
            mRend->bindResource(mHUDMsgTextVB);
            mHUDMsgTextVB->updateData(&vertices[0], vertices.size());
            mRend->drawNonIndexed(vertices.size());
            vertices.clear();
        }
    }

    // Clip Ammo
    sprintf(text, "%d", mPlayerCharacter->getClipAmmo());
    prepareText(text, {bulletHudX + 30, bulletHudY - 5}, 30, vertices);
    mRend->bindResource(mHUDTextAmmoVB);
    mHUDTextAmmoVB->updateData(&vertices[0], vertices.size());
    mRend->drawNonIndexed(vertices.size());
    vertices.clear();

    // Total ammo
    sprintf(text, "%d", mPlayerCharacter->getAmmo());
    prepareText(text, {bulletHudX + 30, bulletHudY + 35}, 30, vertices);
    mRend->bindResource(mHUDTextAmmoVB);
    mHUDTextAmmoVB->updateData(&vertices[0], vertices.size());
    mRend->drawNonIndexed(vertices.size());
    vertices.clear();

    // PopUp Texts
    mRend->bindResource(mHUDMsgTextVB);

    for (PopupText& txt : mPopupTextList) {
        vertices.clear();
        prepareText(txt.mText, txt.mOrigin, txt.mSize, vertices, 1);

        mHUDMsgTextVB->updateData(&vertices[0], vertices.size());
        mRend->drawNonIndexed(vertices.size());
    }
}


