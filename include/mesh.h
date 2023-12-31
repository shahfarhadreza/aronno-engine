#pragma once

class SubMesh
{
protected:
    IGPUResource* mVertexBuffer;
    IGPUIndexBuffer* mIndexBuffer;
    Material* mMaterial;
    AABB mLocalBoundingBox;
public:
    glm::mat4 tempMat;
public:
    SubMesh(IGPUResource* vb, IGPUIndexBuffer* ib);
    IGPUResource* getVertexBuffer() {
        return mVertexBuffer;
    }
    IGPUIndexBuffer* getIndexBuffer() {
        return mIndexBuffer;
    }
    void setMaterial(Material* mat) {
        mMaterial = mat;
    }
    Material* getMaterial() {
        return mMaterial;
    }
    void setLocalBoundingBox(const AABB& aabb) {
        mLocalBoundingBox = aabb;
    }
    const AABB& getLocalBoundingBox() const {
        return mLocalBoundingBox;
    }
};

class SkeletonMesh;

class Mesh : public Resource
{
public:
    std::vector<SubMesh*> mSubMeshList;
    btTriangleMesh* mCollisionMesh;
    AABB mBoundingBox;
public:
    Mesh(const std::string& name)
        : Resource(name), mCollisionMesh(new btTriangleMesh()) {
        // Empty mesh
    }
    virtual ~Mesh() {
        delete mCollisionMesh;
        for(auto it = mSubMeshList.begin();it != mSubMeshList.end();++it) {
            delete *it;
        }
        mSubMeshList.clear();
    }
    SubMesh* createSubMesh(IGPUResource* vb, IGPUIndexBuffer* ib) {
        SubMesh* sm = new SubMesh(vb, ib);
        mSubMeshList.push_back(sm);
        return sm;
    }
    std::vector<SubMesh*>& getSubMeshList() {
        return mSubMeshList;
    }
    btTriangleMesh* getCollisionMesh() {
        return mCollisionMesh;
    }
    void setBoundingBox(const AABB& aabb) {
        mBoundingBox = aabb;
    }
    const AABB& getBoundingBox() const {
        return mBoundingBox;
    }

    virtual SkeletonMesh* isSkeletonMesh() { return nullptr; }
};

class KeyPosition
{
public:
    glm::vec3 position;
    float timeStamp;
};

class KeyRotation
{
public:
    glm::quat orientation;
    float timeStamp;
};

class KeyScale
{
public:
    glm::vec3 scale;
    float timeStamp;
};

class Bone;
class Skeleton;

class BoneAnimationTrack
{
public:
    Bone* mBone;
    std::vector<KeyPosition> mPositions;
    std::vector<KeyRotation> mRotations;
    std::vector<KeyScale> mScales;
public:
    BoneAnimationTrack(Bone* bone)
        : mBone(bone) {
    }

    void insertPositionKey(const KeyPosition& key) {
        mPositions.push_back(key);
    }

    void insertRotationKey(const KeyRotation& key) {
        mRotations.push_back(key);
    }

    void insertScaleKey(const KeyScale& key) {
        mScales.push_back(key);
    }

    /* Gets the current index on mKeyPositions to interpolate to based on
    the current animation time*/
    int GetPositionIndex(float animationTime);
    /* Gets the current index on mKeyRotations to interpolate to based on the
    current animation time*/
    int GetRotationIndex(float animationTime)
    {
        for (size_t index = 0; index < mRotations.size() - 1; ++index)
        {
            if (animationTime < mRotations[index + 1].timeStamp)
                return index;
        }
        return 0;
    }

    /* Gets the current index on mKeyScalings to interpolate to based on the
    current animation time */
    int GetScaleIndex(float animationTime)
    {
        for (size_t index = 0; index < mScales.size() - 1; ++index)
        {
            if (animationTime < mScales[index + 1].timeStamp)
                return index;
        }
        return 0;
    }

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    /*figures out which position keys to interpolate b/w and performs the interpolation
    and returns the translation matrix*/
    glm::mat4 InterpolatePosition(float animationTime);

    /*figures out which rotations keys to interpolate b/w and performs the interpolation
    and returns the rotation matrix*/
    glm::mat4 InterpolateRotation(float animationTime);

    /*figures out which scaling keys to interpolate b/w and performs the interpolation
    and returns the scale matrix*/
    glm::mat4 InterpolateScaling(float animationTime);
};

class SkeletonAnimation
{
public:
    std::string mName;
    std::map<Bone*, BoneAnimationTrack*> mAnimationTrackList;
    float mDuration;
    float mTicks;
public:
    SkeletonAnimation(const std::string& name, float duration, float ticks)
        : mName(name), mDuration(duration), mTicks(ticks)
    {

    }
    virtual ~SkeletonAnimation() {
        for (auto i = mAnimationTrackList.begin(); i != mAnimationTrackList.end(); ++i) {
            delete i->second;
        }
        mAnimationTrackList.clear();
    }

    BoneAnimationTrack* createBoneAnimationTrack(Bone* bone) {
        BoneAnimationTrack* track = new BoneAnimationTrack(bone);
        mAnimationTrackList[bone] = track;
        return track;
    }

    void applyToSkeleton(Skeleton* ske, float timeStamp, float weight);
};

class AnimationState
{
public:
    std::string mName;
    float mLength;
    float mSpeed;
    float mTime;
    bool mLoop;
public:
    AnimationState(const std::string& name, float length);
    void setSpeed(float speed);

    void setTime(float time) { mTime = time; }
    float getTime() const { return mTime; }

    void setLoop(bool loop);

    bool hasEnded() const {
        return (mTime >= mLength && !mLoop);
    }

    void update(float dt);
};

const int MAX_BONES = 200;

class Bone
{
public:
    std::string mName;
    Bone* mParent;
    uint32_t boneId;
    std::vector<Bone*> mChildren;
    std::vector<KeyPosition> mPositions;
    std::vector<KeyRotation> mRotations;
    std::vector<KeyScale> mScales;
    glm::mat4 mLocalTransform;
    glm::mat4 mWorldTransform;
public:
    Bone(const std::string& name, Bone* parent, uint32_t id);
    void updateWorldTransform();

    void setLocalTransform(const glm::mat4& m) { mLocalTransform = m; }
    const glm::mat4& getLocalTransform() { return mLocalTransform; }
    const glm::mat4& getWorldTransform() { return mWorldTransform; }
};

class Skeleton
{
public:
    std::map<std::string, Bone*> mBoneList;
    std::vector<Bone*> mRootBoneList;
    std::map<std::string, SkeletonAnimation*> mAnimationList;
    std::map<std::string, AnimationState*> mAnimationStateList;
    AnimationState* mCurrentAnimState = nullptr;
    glm::mat4 mGlobalInverseTransform;
public:
    Skeleton();
    ~Skeleton();
    Bone* createBone(const std::string& name, uint32_t id, Bone* parent);
    Bone* getBone(const std::string& name);
    void update(float dt);

    SkeletonAnimation* createAnimation(const std::string& name, float length, float ticks);

    AnimationState* getCurrentAnimationState() { return mCurrentAnimState; }
    AnimationState* getAnimationState(const std::string& name);
    AnimationState* setAnimationState(const std::string& name);
    AnimationState* setAnimationState(AnimationState* state);

    void _initAnimationStates();
};

struct BoneInfo {
    /*id is index in finalBoneMatrices*/
    int id;
    /*offset matrix transforms vertex from model space to bone space*/
    glm::mat4 offset;
};

class SkeletonMesh : public Mesh
{
public:
    std::map<std::string, BoneInfo> mBoneInfoMap; //
    int mBoneCounter = 0;
    Skeleton mSkeleton;
public:
    SkeletonMesh(const std::string& name) : Mesh(name) {
    }

    Skeleton* getSkeleton() {
        return &mSkeleton;
    }

    auto& GetBoneInfoMap() { return mBoneInfoMap; }
    int& GetBoneCount() { return mBoneCounter; }

    virtual SkeletonMesh* isSkeletonMesh() { return this; }
};




