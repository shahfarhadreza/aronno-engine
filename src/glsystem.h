#pragma once


// OpenGL Stuffs

struct GLTextureDesc {
    uint32_t Width, Height;
    void* Data;
    bool DoMipMap;
    GLuint InternalFormat;
    GLuint WrapType;
    GLuint Format;
    GLuint DataType;
};

struct GLCubeMapTextureDesc {
    uint32_t Width, Height;
    std::vector<void*> DataList;
    bool DoMipMap;
    GLuint InternalFormat;
    GLuint WrapType;
    GLuint Format;
    GLuint DataType;
};

class GLTexture : public IGPUTexture
{
protected:
    GLuint mTextureId;
    uint32_t mWidth, mHeight;
    bool mCubeMap;
public:
    GLTexture(const GLTextureDesc& desc, bool cubemap);
    GLTexture(const GLCubeMapTextureDesc& desc);
    virtual ~GLTexture();
    virtual uint64_t getResourceId() const { return mTextureId; }
    virtual GPUResourceType getType() const { return GRT_TEXTURE; }

    virtual uint32_t getWidth() const { return mWidth; }
    virtual uint32_t getHeight() const { return mHeight; }

    virtual bool isCubeMap() const { return mCubeMap; }
};

class GLVertexBuffer : public IGPUVertexBuffer
{
protected:
    GLuint mVAO;
    GLuint mVBO;
    uint32_t mVertexCount;
public:
    GLVertexBuffer(const Vertex* data, uint32_t count);
    GLVertexBuffer(const AnimatedVertex* data, uint32_t count);
    virtual ~GLVertexBuffer();
    virtual uint64_t getResourceId() const { return mVBO; }
    virtual GPUResourceType getType() const { return GRT_VERTEX_BUFFER; }

    virtual void updateData(const Vertex* data, uint32_t count);

    GLuint getVertexArrayObject() const { return mVAO; }

    virtual uint32_t getVertexCount() const { return mVertexCount; }
};

class GLIndexBuffer : public IGPUIndexBuffer
{
protected:
    GLuint mBufferId;
    uint32_t mCount;
public:
    GLIndexBuffer(const uint32_t* data, uint32_t count);
    virtual ~GLIndexBuffer();
    virtual uint64_t getResourceId() const { return mBufferId; }
    virtual GPUResourceType getType() const { return GRT_INDEX_BUFFER; }
    virtual uint32_t getIndexCount() const { return mCount; }
};

class GLConstantBuffer : public IGPUConstantBuffer
{
protected:
    GLuint mBufferId;
    uint32_t mSize;
public:
    GLConstantBuffer(uint32_t sizeinBytes);
    virtual ~GLConstantBuffer();
    virtual uint64_t getResourceId() const { return mBufferId; }
    virtual GPUResourceType getType() const { return GRT_CONSTANT_BUFFER; }

    virtual void updateData(void* data);
    virtual uint32_t getBufferSize() const { return mSize; }
};

class GLShader : public IGPUResource
{
protected:
    GLuint mShaderId;
public:
    GLShader(GLenum type, const char* code);
    virtual ~GLShader();

    virtual uint64_t getResourceId() const { return mShaderId; }
    virtual GPUResourceType getType() const { return GRT_SHADER; }
};

class GLProgram : public IGPUShaderProgram
{
protected:
    GLuint mProgramId;
public:
    GLProgram(GLShader* vs, GLShader* ps, GLShader* gs);
    virtual ~GLProgram();

    virtual uint64_t getResourceId() const { return mProgramId; }

    void setValueInt(const char* name, int i);
    void setValueFloat3(const char* name, float x, float y, float z);
    void setValueMatrix4fv(const char* name, float* v);
};

class GLFrameBuffer : public FrameBuffer
{
protected:
    GLuint mBufferId;
    GLTexture* mDepthAttachment;
    std::vector<GLTexture*> mColorAttachmentList;
    FrameBufferDesc mDesc;
public:
    GLFrameBuffer(const FrameBufferDesc& desc);
    virtual ~GLFrameBuffer();

    virtual const FrameBufferDesc& getDescription() const { return mDesc; }
    virtual uint64_t getRenderingId() const { return mBufferId; }
    virtual IGPUTexture* getDepthAttachmentId() const { return mDepthAttachment; }
    virtual IGPUTexture* getColorAttachmentId(uint32_t index) const {
        return mColorAttachmentList[index];
    }

    virtual uint64_t getResourceId() const { return mBufferId; }
};

class OpenGLRenderer : public Renderer
{
protected:
    GLFWwindow* mWindowHandle;
    std::vector<IGPUResource*> mResources;
public:
    OpenGLRenderer(GLFWwindow* windowHandle);
    virtual ~OpenGLRenderer();

    virtual bool init();

    virtual void swapBuffers();

    virtual IGPUShaderProgram* createGPUProgram(IGPUResource* vs, IGPUResource* ps, IGPUResource* gs);
    virtual IGPUTexture* createGPUTexture(int width, int height, void* data, TextureFormat format);
    virtual IGPUTexture* createGPUCubeMapTexture(int width, int height, std::vector<void*> dataList, TextureFormat format);
    virtual IGPUVertexBuffer* createGPUVertexBuffer(const Vertex* data, uint32_t count);
    virtual IGPUVertexBuffer* createGPUAnimatedVertexBuffer(const AnimatedVertex* data, uint32_t count);
    virtual IGPUIndexBuffer* createGPUIndexBuffer(const uint32_t* data, uint32_t count);
    virtual FrameBuffer* createFrameBufferObject(const FrameBufferDesc& desc);

    virtual IGPUConstantBuffer* createGPUConstantBuffer(uint32_t sizeinBytes);

    virtual IGPUResource* createVertexShader(const std::string& code);
    virtual IGPUResource* createPixelShader(const std::string& code);
    virtual IGPUResource* createGeometryShader(const std::string& code);

    virtual void bindConstantBuffer(IGPUConstantBuffer* buffer, enum CBufferBindType type, uint32_t index);

    virtual void bindFrameBuffer(FrameBuffer* fb, const ColorF& color);

    virtual void bindGPUTexture(IGPUTexture* tex, int index);

    virtual void bindResource(IGPUResource* r);
    virtual void unbindResource(IGPUResource* r);

    virtual void setDepthTest(bool enable);
    virtual void setViewport(float left, float top, float width, float height);

    virtual void draw(uint32_t numTriangle);
    virtual void drawInstanced(uint32_t numTriangle, uint32_t numInstance);
    virtual void drawNonIndexed(uint32_t numVertices);
};

GLuint getGLTextureFormat(TextureFormat format);



