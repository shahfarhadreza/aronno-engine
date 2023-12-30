#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "mesh.h"

/* Gets normalized value for Lerp & Slerp*/
float BoneAnimationTrack::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

int BoneAnimationTrack::GetPositionIndex(float animationTime)
{
    //printf("bone (%s): pos size %d\n", mName.c_str(), mPositions.size());
    for (size_t index = 0; index < mPositions.size() - 1; ++index)
    {
        if (animationTime < mPositions[index + 1].timeStamp) {
            return index;
        }
    }
    //printf("bone (%s): no position key found for time stamp %f\n", mName.c_str(), animationTime);
    return 0;
}

/*figures out which position keys to interpolate b/w and performs the interpolation
and returns the translation matrix*/
glm::mat4 BoneAnimationTrack::InterpolatePosition(float animationTime)
{
    if (1 == mPositions.size())
        return glm::translate(glm::mat4(1.0f), mPositions[0].position);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(mPositions[p0Index].timeStamp,
        mPositions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(mPositions[p0Index].position,
        mPositions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

/*figures out which rotations keys to interpolate b/w and performs the interpolation
and returns the rotation matrix*/
glm::mat4 BoneAnimationTrack::InterpolateRotation(float animationTime)
{
    if (1 == mRotations.size())
    {
        auto rotation = glm::normalize(mRotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(mRotations[p0Index].timeStamp,
        mRotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(mRotations[p0Index].orientation,
        mRotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

/*figures out which scaling keys to interpolate b/w and performs the interpolation
and returns the scale matrix*/
glm::mat4 BoneAnimationTrack::InterpolateScaling(float animationTime)
{
    if (1 == mScales.size())
        return glm::scale(glm::mat4(1.0f), mScales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(mScales[p0Index].timeStamp,
        mScales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(mScales[p0Index].scale, mScales[p1Index].scale
        , scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

SubMesh::SubMesh(IGPUResource* vb, IGPUIndexBuffer* ib)
    : mVertexBuffer(vb), mIndexBuffer(ib), mMaterial(nullptr) {
    tempMat = glm::mat4(1.0);
}

Bone::Bone(const std::string& name, Bone* parent, uint32_t id)
    : mName(name), mParent(parent), boneId(id) {
    if (parent) {
        parent->mChildren.push_back(this);
    }
}

void Bone::updateWorldTransform() {
    if (mParent) {
        mWorldTransform = mParent->mWorldTransform * mLocalTransform;
    } else {
        mWorldTransform = mLocalTransform;
    }
    for(Bone* child : mChildren) {
        child->updateWorldTransform();
    }
}

Skeleton::Skeleton() {

}

Skeleton::~Skeleton() {
    for(auto it = mAnimationStateList.begin();it != mAnimationStateList.end();++it) {
        delete it->second;
    }
    mAnimationStateList.clear();
    for(auto it = mAnimationList.begin();it != mAnimationList.end();++it) {
        delete it->second;
    }
    mAnimationList.clear();
    for(auto it = mBoneList.begin();it != mBoneList.end();++it) {
        delete it->second;
    }
    mBoneList.clear();
}

SkeletonAnimation* Skeleton::createAnimation(const std::string& name, float length, float ticks) {
    SkeletonAnimation* a = new SkeletonAnimation(name, length, ticks);
    mAnimationList[name] = a;
    return a;
}

AnimationState* Skeleton::getAnimationState(const std::string& name) {
    return mAnimationStateList[name];
}

AnimationState* Skeleton::setAnimationState(const std::string& name) {
    mCurrentAnimState = mAnimationStateList[name];
    return mCurrentAnimState;
}

AnimationState* Skeleton::setAnimationState(AnimationState* state) {
    mCurrentAnimState = state;
}

Bone* Skeleton::createBone(const std::string& name, uint32_t id, Bone* parent) {
    Bone* b = new Bone(name, parent, id);
    mBoneList[name] = b;
    return b;
}

Bone* Skeleton::getBone(const std::string& name) {
    return mBoneList[name];
}

void Skeleton::_initAnimationStates() {
    for(auto it = mAnimationList.begin();it != mAnimationList.end();++it) {
        SkeletonAnimation* anim = it->second;

        AnimationState* state = new AnimationState(anim->mName, anim->mDuration);
        mAnimationStateList[anim->mName] = state;
    }
}

void Skeleton::update(float dt) {
    if (mCurrentAnimState == nullptr) {
        // grab the first for testing
        for(auto it = mAnimationStateList.begin();it != mAnimationStateList.end();++it) {
            mCurrentAnimState = it->second;
            break;
        }
    }

    if (mCurrentAnimState != nullptr) {
        SkeletonAnimation* anim = mAnimationList[mCurrentAnimState->mName];
        anim->applyToSkeleton(this, mCurrentAnimState->getTime(), 1);

        mCurrentAnimState->update(dt);
    }

    for(Bone* rootBone : mRootBoneList) {
        rootBone->updateWorldTransform();
    }
}

