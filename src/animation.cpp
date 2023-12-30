#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "mesh.h"

void SkeletonAnimation::applyToSkeleton(Skeleton* ske, float timeStamp, float weight) {
    for(auto it = mAnimationTrackList.begin();it != mAnimationTrackList.end();++it) {
        Bone* bone = it->first;
        BoneAnimationTrack* track = it->second;

        glm::mat4 translation = track->InterpolatePosition(timeStamp);
        glm::mat4 rotation = track->InterpolateRotation(timeStamp);
        glm::mat4 scale = track->InterpolateScaling(timeStamp);

        bone->setLocalTransform(translation * rotation * scale);
    }
}

AnimationState::AnimationState(const std::string& name, float length)
    : mName(name), mLength(length), mSpeed(1000), mTime(0), mLoop(false) {

}

void AnimationState::setSpeed(float speed) {
    mSpeed = speed;
}

void AnimationState::setLoop(bool loop) {
    mLoop = loop;
}

void AnimationState::update(float dt) {
    mTime = mTime + mSpeed * dt;
    if (mLoop) {
        // Wrap
        mTime = fmod(mTime, mLength);
        if(mTime < 0) {
            mTime += mLength;
        }
    }
    else {
        // Clamp
        if (mTime < 0) {
            mTime = 0;
        }
        else if (mTime > mLength) {
            mTime = mLength;
        }
    }
}

