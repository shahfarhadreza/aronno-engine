#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "light.h"


Light::Light(bool castShadow)
    : mCastShadow(castShadow) {
    mColor = {1, 1, 1};
    mIntensity = 5;
}

PointLight::PointLight(const glm::vec3& pos, bool castShadow)
    : Light(castShadow), mPosition(pos), mShadowMapFBO(nullptr), mFarPlane(700), mRadius(150), mNeedUpdate(true), mEnabled(true) {

    if (castShadow) {
        Renderer* rnd = Engine::get()->getRenderingSystem();
        // Point light (Cube Shadow Map)
        FrameBufferDesc smDesc;
        smDesc.Width = 1024;
        smDesc.Height = 1024;
        smDesc.Flags = FRAME_BUFFER_FLAG_SHADOW_CUBE;
        smDesc.FilterType = TextureFilterType::PointFilter;
        smDesc.NumRenderTarget = 0;

        mShadowMapFBO = rnd->createFrameBufferObject(smDesc);
    }
    _updateMatrices();
}

AABB PointLight::getBoundingBox() {
    return AABB(getPosition(), getRadius());
}

void PointLight::_updateMatrices() {
    mShadowProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, mFarPlane);

    const auto& lightPos = this->mPosition;

    mShadowViewProjArray[0] = mShadowProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    mShadowViewProjArray[1] = mShadowProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    mShadowViewProjArray[2] = mShadowProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
    mShadowViewProjArray[3] = mShadowProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
    mShadowViewProjArray[4] = mShadowProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
    mShadowViewProjArray[5] = mShadowProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

    mNeedUpdate = false;
}

void PointLight::update(float dt) {
    if (mNeedUpdate) {
        _updateMatrices();
    }
}

DirectionalLight::DirectionalLight(bool castShadow)
    : Light(castShadow), mPrevPosition({0, 0, 0}) {

    //mProjection = glm::ortho(-2000.0f, 2000.0f, -2000.0f, 2000.0f, -100.0f, 3000.0f);
    _updateMatrix();
}

void DirectionalLight::_updateMatrix() {
    //glm::mat4 view = glm::lookAt(mPosition,
                          //glm::vec3( 0.0f, 0.0f,  0.0f),
                          //glm::vec3( 0.0f, 1.0f,  0.0f));

    //mViewProjection = mProjection * view;
}

void DirectionalLight::update(float dt) {
    //if (mPosition != mPrevPosition) {
        //_updateMatrix();
        //mPrevPosition = mPosition;
    //}
}



