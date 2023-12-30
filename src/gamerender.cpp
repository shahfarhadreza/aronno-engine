#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"

#include "game.h"

// gets called only once per frame
void Game::_preparePerFrameData() {
    CameraEntity* cam = mWorld->getViewTarget();

    const TransformComponent& camTransform = mWorld->mTransformComponents[cam->getId()];
    const glm::mat4& camView = glm::inverse(camTransform.Transform);//mCamera->getViewMatrix();
    const glm::mat4& camProj = cam->getProjectionMatrix();

    const auto& lightPos = mSunLight->getDirection();

    mPerFrameData.proj = camProj;
    mPerFrameData.projInverse = glm::inverse(camProj);
    mPerFrameData.view = camView;
    mPerFrameData.viewRotation = glm::mat4(glm::mat3(camView));
    mPerFrameData.cameraPosition = glm::vec4(camTransform.Position, 1);
    mPerFrameData.cameraNear = cam->getNear();
    mPerFrameData.cameraFar = cam->getFar();

    mPerFrameData.screenWidth = SCR_WIDTH;
    mPerFrameData.screenHeight = SCR_HEIGHT;

    mPerFrameData.sunDirection = lightPos;

    mCBPerFrame->updateData(&mPerFrameData);
}

void Game::_prepareLightData() {
    const auto& lightList = mWorld->mPointLightComponents;
    int lightIndex = 0;
    mLightArrayData.lightCount = {0, 0, 0, 0};
    for(auto it = lightList.begin();it != lightList.end();it++) {
        Entity_T entityID = it->first;
        PointLight* pointLight = it->second;

        if (!pointLight->isEnabled()) {
            continue;
        }

        glm::vec3 lightPos = pointLight->getPosition();

        // Also apply transformation to the position if the entity has one
        auto itTrans = mWorld->mWorldTransforms.find(entityID);
        if (itTrans != mWorld->mWorldTransforms.end()) {
            glm::vec4 pos4 = {lightPos.x, lightPos.y, lightPos.z, 1};
            pos4 = itTrans->second * pos4;
            lightPos = {pos4.x, pos4.y, pos4.z};
        }

        // for lighting
        cbPointLight light;
        light.position = glm::vec4(lightPos, pointLight->getFarPlane());
        light.color = glm::vec4(pointLight->getColor(), pointLight->getIntensity());

        if (pointLight->isCastingShadow()) {
            light.direction.x = 1;
        } else {
            light.direction.x = 0;
        }

        mLightArrayData.lights[lightIndex] = light;
        lightIndex++;
    }

    int lightCount = lightIndex;
    mLightArrayData.lightCount = {lightCount, lightCount, lightCount, lightCount};

    mCBLightArray->updateData(&mLightArrayData);
}

glm::mat4 getLightSpaceMatrix(float mShadowMapSize, const glm::mat4& view, float fov, const float nearPlane, const float farPlane)
{
    const auto proj = glm::perspective(
        glm::radians(fov), float(SCR_WIDTH) / float(SCR_HEIGHT), nearPlane, farPlane);

    const glm::mat4 inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : frustumCorners)
    {
        center += glm::vec3(v);
    }
    center /= 8.0f;

    //glm::vec3 lightDir = glm::normalize(mRenderer->GetLightDirection());
   /// glm::vec3 lightPos = center + lightDir * (farPlane - nearPlane);
    //glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3 lightDir = glm::normalize(World::get()->mSunLight->getDirection());

    const auto lightView = glm::lookAt(center, center + lightDir, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : frustumCorners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
/*
    glm::mat4 shadowMatrix = lightProjection * lightView;
    glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    shadowOrigin = shadowMatrix * shadowOrigin;
    shadowOrigin = shadowOrigin * mShadowMapSize / 2.0f;

    glm::vec4 roundedOrigin = glm::round(shadowOrigin);
    glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
    roundOffset = roundOffset * 2.0f / mShadowMapSize;
    roundOffset.z = 0.0f;
    roundOffset.w = 0.0f;

    glm::mat4 shadowProj = lightProjection;
    shadowProj[3] += roundOffset;
    lightProjection = shadowProj;
*/
    return lightProjection * lightView;
}

std::vector<glm::mat4> getLightSpaceMatrices(CameraEntity* camera, const std::vector<float>& shadowCascadeLevels, const glm::mat4& view)
{
    const float cameraNearPlane = camera->getNear();
    const float cameraFarPlane = camera->getFar();
    const float fov = camera->getFOV();

    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(4096.0f, view, fov, cameraNearPlane, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(3048.0f, view, fov, shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(4096.0f, view, fov, shadowCascadeLevels[i - 1], cameraFarPlane));
        }
    }
    return ret;
}

int totalDraw = 0;

bool show_another_window = true;

void Game::renderScene(enum RenderPassType pass, Frustum* frustum) {

    const auto& meshCompList = mWorld->mMeshComponents;

    for(auto itEnt = meshCompList.begin(); itEnt != meshCompList.end();++itEnt) {
        const MeshComponent& comp = itEnt->second;
        Mesh* mesh = comp.mMesh;
        Entity_T entityID = itEnt->first;

        SkeletonMesh* skeMesh = mesh->isSkeletonMesh();
        if (skeMesh) {
            Skeleton* skeleton = skeMesh->getSkeleton();

            for(auto it = skeMesh->mBoneInfoMap.begin(); it != skeMesh->mBoneInfoMap.end();++it){
                const BoneInfo& boneInfo = it->second;

                Bone* bone = skeleton->getBone(it->first);
                assert(boneInfo.id < MAX_BONES);
                mPerAnimatedObjectData.gBones[boneInfo.id] = bone->mWorldTransform * boneInfo.offset;
            }
            mCBPerAnimatedObject->updateData(&mPerAnimatedObjectData);

            mPerObjectData.animated = 1;
        } else {
            mPerObjectData.animated = 0;
        }
        auto itTrans = mWorld->mWorldTransforms.find(entityID);
        if (itTrans != mWorld->mWorldTransforms.end()) {
            mPerObjectData.world = itTrans->second;
        } else {
            mPerObjectData.world = MatIdent;
        }
        mCBPerObject->updateData(&mPerObjectData);

        const auto& sml = mesh->getSubMeshList();

        for (auto it = sml.begin(); it != sml.end();++it) {
            SubMesh* sm = *it;
            IGPUIndexBuffer* ib = sm->getIndexBuffer();
            Material* mat = sm->getMaterial();

            if (pass == RenderPassType::DepthPass) {
                if (mat->isTwoSided()) {
                    //continue;
                }
            }

            AABB bb = sm->getLocalBoundingBox();
            bb.transform(mPerObjectData.world);

            bool isVisibleToFrustum = false;

            // BUG: ????!
            if (skeMesh) {
                isVisibleToFrustum = true;
            } else {
                isVisibleToFrustum = frustum->IsBoxVisible(bb.getMin(), bb.getMax());
            }

            if (isVisibleToFrustum) {
                Texture* dmap = mat->getDiffuseMap();
                if (dmap) {
                    auto gpur = dmap->getGPUResource();
                    mRend->bindGPUTexture(gpur, 1);
                    /*
                    if (mat->isTwoSided()) {
                        mPerObjectData.isTransparent = 1;
                    }
                    */
                }
                mRend->bindResource(sm->getVertexBuffer());
                mRend->bindResource(ib);
                mRend->draw(ib->getIndexCount());
                totalDraw++;
            }
        }
    }
}

void Game::_bindShaders() {
    // let's provide the per frame data to both vertex & pixel shaders
    mRend->bindConstantBuffer(mCBPerFrame, CBBT_VS, 0);
    mRend->bindConstantBuffer(mCBPerFrame, CBBT_PS, 0);

    mRend->bindConstantBuffer(mCBPerObject, CBBT_VS, 1);
    mRend->bindConstantBuffer(mCBPerObject, CBBT_PS, 1);

    mRend->bindConstantBuffer(mCBLightArray, CBBT_VS, 2);
    mRend->bindConstantBuffer(mCBLightArray, CBBT_PS, 2);

    mRend->bindConstantBuffer(mCBShadowCube, CBBT_GS, 3);
    mRend->bindConstantBuffer(mCBShadowCube, CBBT_PS, 3);

    // SSAO samples data
    mRend->bindConstantBuffer(mCBSSAO, CBBT_VS, 4);
    mRend->bindConstantBuffer(mCBSSAO, CBBT_PS, 4);

    mRend->bindConstantBuffer(mCBPostProcess, CBBT_VS, 5);
    mRend->bindConstantBuffer(mCBPostProcess, CBBT_PS, 5);

    mRend->bindConstantBuffer(mCBCascadedShadowProj, CBBT_VS, 6);
    mRend->bindConstantBuffer(mCBCascadedShadowProj, CBBT_PS, 6);

    mRend->bindConstantBuffer(mCBCascadedShadow, CBBT_VS, 7);
    mRend->bindConstantBuffer(mCBCascadedShadow, CBBT_PS, 7);

    // animation bones data buffer
    mRend->bindConstantBuffer(mCBPerAnimatedObject, CBBT_VS, 8);
}

void Game::render() {
/*
    if (mCurrentState) {
        mCurrentState->render();
    }
*/
    _preparePerFrameData();

    // We just need to bind them for once
    if (mBindConstBuffers) {
        _bindShaders();
        mBindConstBuffers = false;
    }

    glm::mat4 model = glm::mat4(1.0f);

    // also provide the point light array
    _prepareLightData();

    mPerObjectData.world = model;
    mPerObjectData.hasNormalMap = 0;
    mPerObjectData.hasEmissionMap = 0;
    mPerObjectData.specularIntensity = 0;
    mPerObjectData.animated = 0;
    mCBPerObject->updateData(&mPerObjectData);

    totalDraw = 0;

    Frustum mainCameraFrustum(mPerFrameData.proj * mPerFrameData.view);

    const auto& meshCompList = mWorld->mMeshComponents;

    // Depth pass (SSAO)
    if (mPerFrameData.enableSSAO == 1) {
        mRend->bindFrameBuffer(depthFBO, {1.0f, 1.0f, 1.0f, 1.0f});
        mRend->bindResource(depthProgram);

        renderScene(RenderPassType::DepthPass, &mainCameraFrustum);
    }

    if (mPerFrameData.sunEnableShadow) {
        const float cameraFarPlane = mCamera->getFar();
        std::vector<float> shadowCascadeLevels{ cameraFarPlane / 15, cameraFarPlane / 5, cameraFarPlane };

        auto lightMatrics = getLightSpaceMatrices(mCamera, shadowCascadeLevels, mPerFrameData.view);

        mCascadedShadowData.cascadePlaneDistances.x = shadowCascadeLevels[0];
        mCascadedShadowData.cascadePlaneDistances.y = shadowCascadeLevels[1];
        mCascadedShadowData.cascadePlaneDistances.z = shadowCascadeLevels[2];
        mCascadedShadowData.cascadePlaneDistances.w = 0;

        mCascadedShadowData.splits[0] = lightMatrics[0];
        mCascadedShadowData.splits[1] = lightMatrics[1];
        mCascadedShadowData.splits[2] = lightMatrics[2];

        mCBCascadedShadow->updateData(&mCascadedShadowData);

        // Direction/Sun light depth pass for cascaded shadow map...
        mRend->bindResource(shadowDepthProgram);

        cbCascadedShadowProj shadowProj;
        Frustum cascadedFrustum;

        mRend->bindFrameBuffer(mCascadedFBOSplit1, {1.0f, 1.0f, 1.0f, 1.0f});
        shadowProj.lightProjView = mCascadedShadowData.splits[0];
        mCBCascadedShadowProj->updateData(&shadowProj);

        cascadedFrustum = Frustum(shadowProj.lightProjView);
        renderScene(RenderPassType::SunShadowPass, &cascadedFrustum);

        mRend->bindFrameBuffer(mCascadedFBOSplit2, {1.0f, 1.0f, 1.0f, 1.0f});
        shadowProj.lightProjView = mCascadedShadowData.splits[1];
        mCBCascadedShadowProj->updateData(&shadowProj);

        cascadedFrustum = Frustum(shadowProj.lightProjView);
        renderScene(RenderPassType::SunShadowPass, &cascadedFrustum);

        mRend->bindFrameBuffer(mCascadedFBOSplit3, {1.0f, 1.0f, 1.0f, 1.0f});
        shadowProj.lightProjView = mCascadedShadowData.splits[2];
        mCBCascadedShadowProj->updateData(&shadowProj);

        cascadedFrustum = Frustum(shadowProj.lightProjView);
        renderScene(RenderPassType::SunShadowPass, &cascadedFrustum);
    }

    // Cube depth map
    const auto& lightList = mWorld->mPointLightComponents;

    if (lightList.size() > 0) {
        cbShadowCube data;

        mRend->bindResource(shadowCubeDepthProgram);

        int totalVisbleLight = 0;

        for(auto it = lightList.begin();it != lightList.end();it++) {
            PointLight* pointLight = it->second;

            if (pointLight->isEnabled() && pointLight->isCastingShadow()) {

                const AABB lightBB = pointLight->getBoundingBox();

                const glm::vec3& lightPos = pointLight->getPosition();

                data.lightPos = glm::vec4(lightPos, pointLight->getFarPlane());

                for (int m = 0;m < 6;m++) {
                    data.shadowMatrices[m] = pointLight->getShadowViewProj(m);
                }

                mCBShadowCube->updateData(&data);

                mRend->bindFrameBuffer(pointLight->getShadowMapFBO(), {1.0f, 1.0f, 1.0f, 1.0f});

                if (mainCameraFrustum.IsBoxVisible(lightBB.getMin(), lightBB.getMax())) {
                    totalVisbleLight++;

                    for(auto itEnt = meshCompList.begin(); itEnt != meshCompList.end();++itEnt) {
                        const MeshComponent& comp = itEnt->second;
                        Mesh* mesh = comp.mMesh;
                        Entity_T entityID = itEnt->first;

                        SkeletonMesh* skeMesh = mesh->isSkeletonMesh();
                        if (skeMesh) {
                            Skeleton* skeleton = skeMesh->getSkeleton();

                            for(auto it = skeMesh->mBoneInfoMap.begin(); it != skeMesh->mBoneInfoMap.end();++it){
                                const BoneInfo& boneInfo = it->second;

                                Bone* bone = skeleton->getBone(it->first);
                                assert(boneInfo.id < MAX_BONES);
                                mPerAnimatedObjectData.gBones[boneInfo.id] = bone->mWorldTransform * boneInfo.offset;
                            }
                            mCBPerAnimatedObject->updateData(&mPerAnimatedObjectData);

                            mPerObjectData.animated = 1;
                        } else {
                            mPerObjectData.animated = 0;
                        }
                        auto itTrans = mWorld->mWorldTransforms.find(entityID);
                        if (itTrans != mWorld->mWorldTransforms.end()) {
                            mPerObjectData.world = itTrans->second;
                        } else {
                            mPerObjectData.world = model;
                        }
                        mCBPerObject->updateData(&mPerObjectData);

                        const auto& sml = mesh->getSubMeshList();

                        for (auto it = sml.begin(); it != sml.end();++it) {
                            SubMesh* sm = *it;

                            INTERSECTION_TYPE result = sm->getLocalBoundingBox().intersect(lightBB);

                            if (result != INTERSECTION_TYPE::OUTSIDE) {
                                IGPUIndexBuffer* ib = sm->getIndexBuffer();
                                mRend->bindResource(sm->getVertexBuffer());
                                mRend->bindResource(ib);
                                mRend->draw(ib->getIndexCount());
                            }
                        }
                    }
                }
            }
        }
    }
    // Lighting Pass
    mRend->bindFrameBuffer(primaryFBO, {1.0f, 1.0f, 1.0f, 1.0f});

    // Draw The Sky
    mRend->bindResource(skyProgram);
    mRend->bindGPUTexture(mSkyTexture->getGPUResource(), 3);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    mRend->setDepthTest(false);

    mRend->bindResource(mSkyBoxVB);
    mRend->drawNonIndexed(mSkyBoxVB->getVertexCount());

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    mRend->setDepthTest(true);


    // Draw Scene

    mRend->bindResource(lightProgram);

    if (mPerFrameData.sunEnableShadow) {
        // Directional/Sun Light Cascaded Shadow Map
        IGPUTexture* split1 = mCascadedFBOSplit1->getDepthAttachmentId();
        IGPUTexture* split2 = mCascadedFBOSplit2->getDepthAttachmentId();
        IGPUTexture* split3 = mCascadedFBOSplit3->getDepthAttachmentId();

        mRend->bindGPUTexture(split1, 6);
        mRend->bindGPUTexture(split2, 7);
        mRend->bindGPUTexture(split3, 8);
    }

    // First, bind all the scene lights depth map textures (from the cube depth pass)
    // so that we can use them to project shadows in our lighting shader/pass
    int cubeShadowMapStartIndex = 9;
    for(auto it = lightList.begin();it != lightList.end();it++) {
        PointLight* pointLight = it->second;

        if (pointLight->isEnabled() && pointLight->isCastingShadow()) {

            IGPUTexture* tex = pointLight->getShadowMapFBO()->getDepthAttachmentId();

            mRend->bindGPUTexture(tex, cubeShadowMapStartIndex);
            cubeShadowMapStartIndex++;
        }
    }

    // Draw level (Only Solid Stuffs)

    for(auto itEnt = meshCompList.begin(); itEnt != meshCompList.end();++itEnt) {
        const MeshComponent& comp = itEnt->second;
        Mesh* mesh = comp.mMesh;
        Entity_T entityID = itEnt->first;

        SkeletonMesh* skeMesh = mesh->isSkeletonMesh();
        auto itTrans = mWorld->mWorldTransforms.find(entityID);
        if (itTrans != mWorld->mWorldTransforms.end()) {
            mPerObjectData.world = itTrans->second;
        } else {
            mPerObjectData.world = model;
        }

        const auto& sml = mesh->getSubMeshList();

        for (auto it = sml.begin(); it != sml.end();++it) {
            SubMesh* sm = *it;
            Material* mat = sm->getMaterial();

            if (mat->isTwoSided()) {
                continue;
            }

            if (skeMesh) {
                Skeleton* skeleton = skeMesh->getSkeleton();
                for(auto it = skeMesh->mBoneInfoMap.begin(); it != skeMesh->mBoneInfoMap.end();++it){
                    const BoneInfo& boneInfo = it->second;

                    Bone* bone = skeleton->getBone(it->first);
                    assert(boneInfo.id < MAX_BONES);
                    mPerAnimatedObjectData.gBones[boneInfo.id] = glm::inverse(sm->tempMat) * bone->getWorldTransform() * boneInfo.offset;
                }
                mCBPerAnimatedObject->updateData(&mPerAnimatedObjectData);

                mPerObjectData.animated = 1;
            } else {
                mPerObjectData.animated = 0;
            }

            AABB bb = sm->getLocalBoundingBox();
            bb.transform(mPerObjectData.world);

            bool isVisibleToCam = false;

            // BUG: ????!
            if (skeMesh) {
                isVisibleToCam = true;
            } else {
                isVisibleToCam = mainCameraFrustum.IsBoxVisible(bb.getMin(), bb.getMax());
            }

            if (isVisibleToCam) {

                mPerObjectData.hasNormalMap = 0;
                mPerObjectData.hasEmissionMap = 0;

                mPerObjectData.specularIntensity = mat->getSpecularColor().red;

                if (mat) {

                    Texture* dmap = mat->getDiffuseMap();
                    if (dmap) {
                        auto gpur = dmap->getGPUResource();
                        mRend->bindGPUTexture(gpur, 1);
                    }
                    Texture* nmap = mat->getNormalMap();
                    if (nmap) {
                        auto gpur = nmap->getGPUResource();
                        mRend->bindGPUTexture(gpur, 2);
                        mPerObjectData.hasNormalMap = 1;
                    }
                    Texture* emap = mat->getEmissionMap();
                    if (emap) {
                        auto gpur = emap->getGPUResource();
                        mRend->bindGPUTexture(gpur, 3);
                        mPerObjectData.hasEmissionMap = 1;
                    }

                    if (mat->mMetalnessMap) {
                        auto gpur = mat->mMetalnessMap->getGPUResource();
                        mRend->bindGPUTexture(gpur, 4);
                        //mPerObjectData.hasMetalnessMap = 1;
                    }

                    if (mat->mRoughnessMap) {
                        auto gpur = mat->mRoughnessMap->getGPUResource();
                        mRend->bindGPUTexture(gpur, 5);
                        //mPerObjectData.hasRoughnessMap = 1;
                    }
                }

                mCBPerObject->updateData(&mPerObjectData);

                IGPUIndexBuffer* ib = sm->getIndexBuffer();
                mRend->bindResource(sm->getVertexBuffer());
                mRend->bindResource(ib);
                mRend->draw(ib->getIndexCount());
                totalDraw++;
            }
        }
    }

    // Draw level (Only Transparent Stuffs)
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for(auto itEnt = meshCompList.begin(); itEnt != meshCompList.end();++itEnt) {
        const MeshComponent& comp = itEnt->second;
        Mesh* mesh = comp.mMesh;
        Entity_T entityID = itEnt->first;

        mPerObjectData.animated = 0;
        auto itTrans = mWorld->mWorldTransforms.find(entityID);
        if (itTrans != mWorld->mWorldTransforms.end()) {
            mPerObjectData.world = itTrans->second;
        } else {
            mPerObjectData.world = model;
        }

        const auto& sml = mesh->getSubMeshList();

        for (auto it = sml.begin(); it != sml.end();++it) {
            SubMesh* sm = *it;
            Material* mat = sm->getMaterial();

            if (!mat->isTwoSided()) {
                continue;
            }

            mPerObjectData.hasNormalMap = 0;
            mPerObjectData.hasEmissionMap = 0;
            mPerObjectData.specularIntensity = mat->getSpecularColor().red;

            if (mat) {

                Texture* dmap = mat->getDiffuseMap();
                if (dmap) {
                    auto gpur = dmap->getGPUResource();
                    mRend->bindGPUTexture(gpur, 1);
                }
                Texture* nmap = mat->getNormalMap();
                if (nmap) {
                    auto gpur = nmap->getGPUResource();
                    mRend->bindGPUTexture(gpur, 2);
                    mPerObjectData.hasNormalMap = 1;
                }
                Texture* emap = mat->getEmissionMap();
                if (emap) {
                    auto gpur = emap->getGPUResource();
                    mRend->bindGPUTexture(gpur, 3);
                    mPerObjectData.hasEmissionMap = 1;
                }
            }

            mCBPerObject->updateData(&mPerObjectData);

            IGPUIndexBuffer* ib = sm->getIndexBuffer();
            mRend->bindResource(sm->getVertexBuffer());
            mRend->bindResource(ib);
            mRend->draw(ib->getIndexCount());
            totalDraw++;
        }
    }

    sprintf(debugText, "sub mesh rendered %d", totalDraw);

    // Draw all the billboards too
    const auto& billboradCompList = mWorld->mBillboardComponents;

    mRend->bindResource(fxProgram);
    mRend->bindQuadBuffer(mMuzzleQuad);

    for (auto it = billboradCompList.begin();it != billboradCompList.end();it++) {
        const BillboardComponent& billoard = it->second;
        Entity_T entityID = it->first;

        auto itTrans = mWorld->mWorldTransforms.find(entityID);
        if (itTrans != mWorld->mWorldTransforms.end()) {
            mPerObjectData.world = itTrans->second;
        } else {
            mPerObjectData.world = glm::mat4(1.0);
        }

        mPerObjectData.opacity = billoard.Opacity;
        mCBPerObject->updateData(&mPerObjectData);

        mRend->bindGPUTexture(billoard.Image->getGPUResource(), 3);

        mRend->draw(mMuzzleQuad->getIndexCount());
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
/*
    glm::mat4 matObject(1.0);

    // Draw Some Debug Stuffs
    mRend->bindResource(fxProgram);
    mRend->bindResource(mBoneVB);
    mRend->bindResource(mBoneIB);
    mRend->setDepthTest(false);
    for(auto itEnt = meshCompList.begin(); itEnt != meshCompList.end();++itEnt) {
        const MeshComponent& comp = itEnt->second;
        Mesh* mesh = comp.mMesh;
        Entity_T entityID = itEnt->first;

        auto itTrans = mWorld->mWorldTransforms.find(entityID);
        if (itTrans != mWorld->mWorldTransforms.end()) {
            matObject = itTrans->second;
        }

        SkeletonMesh* skeMesh = mesh->isSkeletonMesh();
        if (skeMesh) {
            Skeleton* skeleton = skeMesh->getSkeleton();

            for(auto it = skeleton->mBoneList.begin(); it != skeleton->mBoneList.end();++it){
                Bone* bone = it->second;

                glm::vec3 size = glm::vec3(0.2, 1, 0.2);
                glm::vec3 center2 = glm::vec3(0, 0, 0);
                glm::mat4 transform = glm::translate(glm::mat4(1), center2) * glm::scale(glm::mat4(1), size);

                mPerObjectData.world = bone->mWorldTransform * transform;
                mPerObjectData.opacity = 1;
                mCBPerObject->updateData(&mPerObjectData);

                glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
                glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid*)(4*sizeof(GLuint)));
                glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid*)(8*sizeof(GLuint)));
            }
        }
    }
    mRend->setDepthTest(true);


    // Draw Some Debug Stuffs
    mRend->bindResource(fxProgram);
    mRend->bindResource(mBoundingBoxVB);
    mRend->bindResource(mBoundingBoxIB);

    for(auto itEnt = meshCompList.begin(); itEnt != meshCompList.end();++itEnt) {
        const MeshComponent& comp = itEnt->second;
        Mesh* mesh = comp.mMesh;
        Entity_T entityID = itEnt->first;

        glm::mat4 bbTransform = glm::mat4(1.0f);
        char hasWorldTransform = 0;

        auto itTrans = mWorld->mWorldTransforms.find(entityID);
        if (itTrans != mWorld->mWorldTransforms.end()) {
            bbTransform = itTrans->second;
        }

        const auto& sml = mesh->getSubMeshList();

        for(auto it = sml.begin(); it != sml.end();++it){
            SubMesh* sm = *it;
            AABB bb = sm->getLocalBoundingBox();

            bb.transform(bbTransform);

            float min_x = bb.getMin().x;
            float min_y = bb.getMin().y;
            float min_z = bb.getMin().z;
            float max_x = bb.getMax().x;
            float max_y = bb.getMax().y;
            float max_z = bb.getMax().z;

            glm::vec3 size = glm::vec3(max_x-min_x, max_y-min_y, max_z-min_z);
            glm::vec3 center = glm::vec3((min_x+max_x)/2, (min_y+max_y)/2, (min_z+max_z)/2);
            glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);

            mPerObjectData.world = transform;
            mPerObjectData.opacity = 1;
            mCBPerObject->updateData(&mPerObjectData);

            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid*)(4*sizeof(GLuint)));
            glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid*)(8*sizeof(GLuint)));
        }
    }
*/
/*
    // Draw point lights bounding box
    for(auto it = lightList.begin();it != lightList.end();it++) {
        PointLight* pointLight = (*it)->isPointLight();

        const AABB bb = pointLight->getBoundingBox();
        float min_x = bb.getMin().x;
        float min_y = bb.getMin().y;
        float min_z = bb.getMin().z;
        float max_x = bb.getMax().x;
        float max_y = bb.getMax().y;
        float max_z = bb.getMax().z;

        glm::vec3 size = glm::vec3(max_x-min_x, max_y-min_y, max_z-min_z);
        glm::vec3 center2 = glm::vec3((min_x+max_x)/2, (min_y+max_y)/2, (min_z+max_z)/2);
        glm::mat4 transform = glm::translate(glm::mat4(1), center2) * glm::scale(glm::mat4(1), size);

        mPerObjectData.world = transform;
        mPerObjectData.opacity = 1;
        mCBPerObject->updateData(&mPerObjectData);

        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid*)(4*sizeof(GLuint)));
        glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid*)(8*sizeof(GLuint)));
    }
*/


    // Draw Bullet Projectiles
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mRend->bindResource(projectileProgram);
    mRend->bindResource(mProjectileVB);
    mRend->bindResource(mProjectileIB);

    mRend->bindGPUTexture(mProjectileTexture, 3);

    for (BulletProjectile& bullet : mBulletProjectiles) {
        mPerObjectData.world = bullet.mTransform;
        mCBPerObject->updateData(&mPerObjectData);
        mRend->draw(mProjectileIB->getIndexCount());
    }

    mRend->bindGPUTexture(mBulletDecalTexture->getGPUResource(), 3);

    // Draw Decals
    for(auto it = mDecals.begin(); it != mDecals.end();it++) {
        Decal& decal = *it;
        mPerObjectData.world = decal.mTransform;
        mCBPerObject->updateData(&mPerObjectData);
        mRend->draw(mProjectileIB->getIndexCount());
    }

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);


    // SSAO Pass
    IGPUTexture* ssaoTexture = nullptr;

    if (mPerFrameData.enableSSAO == 1) {
        mRend->bindFrameBuffer(ssaoFBO, {1.0f, 1.0f, 1.0f, 1.0f});

        mRend->bindResource(ssaoProgram);

        IGPUTexture* fboDepthTexture = depthFBO->getDepthAttachmentId();
        mRend->bindGPUTexture(fboDepthTexture, 4);
        mRend->bindGPUTexture(mNoiseTexture, 5);

        mRend->bindQuadBuffer(mScreenQuad);
        mRend->draw(mScreenQuad->getIndexCount());

        ssaoTexture = ssaoFBO->getColorAttachmentId(0);
    }

    // Other passes
    IGPUTexture* primaryFboTexture = primaryFBO->getColorAttachmentId(0);
    IGPUTexture* brightpassTexture = primaryFBO->getColorAttachmentId(1);
    //IGPUTexture* primaryBlackTexture = primaryFBO->getColorAttachmentId(2);

    // For Bloom
    IGPUTexture* bloompassTexture = nullptr;
    if (mPerFrameData.postEnableBloom == 1) {
        // Blur Pass 1
        mPostProcessData.horizontalPass = 0;
        mCBPostProcess->updateData(&mPostProcessData);

        mRend->bindFrameBuffer(blurPassFBO1, {1.0f, 1.0f, 1.0f, 1.0f});

        mRend->bindResource(blurProgram);
        mRend->bindGPUTexture(brightpassTexture, 0);

        mRend->bindQuadBuffer(mScreenQuad);
        mRend->draw(mScreenQuad->getIndexCount());

        IGPUTexture* vblurpassTexture = blurPassFBO1->getColorAttachmentId(0);

        // Blur Pass 2
        mPostProcessData.horizontalPass = 1;
        mCBPostProcess->updateData(&mPostProcessData);

        mRend->bindFrameBuffer(blurPassFBO2, {1.0f, 1.0f, 1.0f, 1.0f});

        mRend->bindResource(blurProgram);
        mRend->bindGPUTexture(vblurpassTexture, 0);

        mRend->bindQuadBuffer(mScreenQuad);
        mRend->draw(mScreenQuad->getIndexCount());

        bloompassTexture = blurPassFBO2->getColorAttachmentId(0);
    }

    // Final Pass
    mRend->bindFrameBuffer(0, {1.0f, 0.0f, 0.0f, 1.0f});

    mRend->bindResource(quadProgram);
    mRend->bindGPUTexture(primaryFboTexture, 0);
    if (mPerFrameData.postEnableBloom == 1) {
        mRend->bindGPUTexture(bloompassTexture, 1);
    }
    if (mPerFrameData.enableSSAO == 1) {
        mRend->bindGPUTexture(ssaoTexture, 2);
    }
    //mRend->bindGPUTexture(primaryBlackTexture, 3);

    mRend->bindQuadBuffer(mScreenQuad);
    mRend->draw(mScreenQuad->getIndexCount());

    // Draw hud elements (on top of everything)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    renderHUD();

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderGUI();

    // GUI Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    mRend->swapBuffers();
}

void Game::renderGUI() {
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::Begin("Settings", &show_another_window);

    ImGui::Text("SSAO - ");
    ImGui::Checkbox("Enable SSAO", (bool*)&this->mPerFrameData.enableSSAO);
    ImGui::SliderFloat("Distance", &this->mPerFrameData.SSAODistance, -5.0f, 5.0f);
    ImGui::SliderFloat("Distance Power", &this->mPerFrameData.SSAODistancePower, -5.0f, 5.0f);
    ImGui::SliderFloat("Move Amount", &this->mPerFrameData.SSAOMoveAmount, -5.0f, 5.0f);

    ImGui::Text("Sun Light - ");
    ImGui::SliderFloat3("Direction", this->mSunLight->getDirectionPtr(), -1.0f, 1.0f);
    ImGui::SliderFloat("Intensity", &this->mPerFrameData.sunlightIntensity, 0.0f, 50.0f);
    ImGui::SliderFloat("Indirect Intensity", &this->mPerFrameData.sunIndirectIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Ambient", &this->mPerFrameData.sunlightAmbient, 0.0f, 1.0f);
    ImGui::Checkbox("Enable Shadow", (bool*)&this->mPerFrameData.sunEnableShadow);

    ImGui::Text("Post-Process");
    ImGui::SliderFloat("Saturation", &this->mPerFrameData.postSaturation, 0.0f, 2.0f);
    ImGui::Checkbox("Enable Bloom", (bool*)&this->mPerFrameData.postEnableBloom);
    ImGui::SliderFloat("Bloom Amount", &this->mPerFrameData.postBloomIntensity, 0.0f, 1.0f);
    ImGui::Checkbox("Enable ToneMapping", (bool*)&this->mPerFrameData.postEnableToneMapping);
    ImGui::SliderFloat("Exposure", &this->mPerFrameData.postExposure, 0.0f, 10.0f);
    ImGui::SliderFloat("Max Bright", &this->mPerFrameData.postbBrightMax, 0.0f, 50.0f);

    ImGui::Text("Atmosphere");
    ImGui::SliderFloat("Fog Density", &this->mPerFrameData.fogDensity, 0.0f, 1.0f);

    ImGui::Text("Extras");
    ImGui::SliderFloat("Metalic", &this->mPerFrameData.metallic, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &this->mPerFrameData.roughness, 0.0f, 1.0f);

    ImGui::End();
}













