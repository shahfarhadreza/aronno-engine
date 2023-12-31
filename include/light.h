#pragma once

class DirectionalLight;
class PointLight;
class FrameBuffer;

class Light
{
protected:
    std::string mName;
    glm::vec3 mColor;
    bool mCastShadow;
    float mIntensity;
public:
    Light(bool castShadow);
    virtual ~Light() { }
    void setName(const std::string& name) {
        mName = name;
    }
    const std::string& getName() const {
        return mName;
    }
    void setColor(const glm::vec3& color) {
        mColor = color;
    }
    const glm::vec3& getColor() const {
        return mColor;
    }
    bool isCastingShadow() const { return mCastShadow; }
    void enableShadowCasting(bool enable) {
        mCastShadow = enable;
    }

    void setIntensity(float intensity) { mIntensity = intensity; }
    float getIntensity() const { return mIntensity; }

    virtual DirectionalLight* isDirectionalLight() { return nullptr; }
    virtual PointLight* isPointLight() { return nullptr; }

    virtual void update(float dt) { }
};

class DirectionalLight : public Light
{
protected:
    glm::vec3 mPrevPosition;
    glm::vec3 mDirection;
    glm::mat4 mProjection;
    glm::mat4 mViewProjection;
public:
    DirectionalLight(bool castShadow);
    const glm::vec3& getDirection() const { return mDirection; }
    float* getDirectionPtr() { return &mDirection.x; }
    const void setDirection(const glm::vec3& dir) {
        mDirection = dir;
    }

    const glm::mat4& getMatrix() const { return mViewProjection; }

    virtual DirectionalLight* isDirectionalLight() { return this; }

    virtual void update(float dt);
protected:
    void _updateMatrix();
};

class PointLight : public Light
{
protected:
    glm::vec3 mPosition;
    FrameBuffer* mShadowMapFBO;
    float mFarPlane;
    glm::mat4 mShadowProjection;
    glm::mat4 mShadowViewProjArray[6];
    bool mNeedUpdate;
    float mRadius;
    bool mEnabled;
public:
    PointLight(const glm::vec3& pos, bool castShadow);
    const glm::vec3& getPosition() const {
        return mPosition;
    }
    void setPosition(const glm::vec3& pos) {
        mPosition = pos;
        mNeedUpdate = true;
    }
    void setEnabled(bool enable) { mEnabled = enable; }
    bool isEnabled() const { return mEnabled; }
    FrameBuffer* getShadowMapFBO() { return mShadowMapFBO; }
    virtual PointLight* isPointLight() { return this; }
    float getFarPlane() const { return mFarPlane; }
    float getRadius() const { return mRadius; }

    const glm::mat4& getShadowViewProj(int index) const { return mShadowViewProjArray[index]; }

    AABB getBoundingBox();

    virtual void update(float dt);
protected:
    void _updateMatrices();
};


