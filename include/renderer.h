#pragma once

class IGPUShaderProgram;
class IGPUConstantBuffer;
class SubMesh;
class Mesh;
class SkeletonMesh;
class Vertex;
class FrameBuffer;

class Resource
{
protected:
    std::string mName;
    std::string mFilePath;
public:
    Resource(const std::string& name, const std::string& path)
        : mName(name), mFilePath(path) {
    }
    Resource(const std::string& name)
        : mName(name), mFilePath("") {
    }
    virtual ~Resource() { }

    const std::string& getName() const { return mName; }
};

enum GPUResourceType {
    GRT_INVALID,
    GRT_TEXTURE,
    GRT_SHADER,
    GRT_SHADER_PROGRAM,
    GRT_VERTEX_BUFFER,
    GRT_INDEX_BUFFER,
    GRT_CONSTANT_BUFFER,
    GRT_FRAMEBUFFER,
};

class IGPUResource
{
public:
    IGPUResource() { }
    virtual ~IGPUResource() { }
    virtual uint64_t getResourceId() const = 0;
    virtual GPUResourceType getType() const = 0;
};

class IGPUTexture : public IGPUResource
{
public:
    virtual uint32_t getWidth() const = 0;
    virtual uint32_t getHeight() const = 0;
    virtual bool isCubeMap() const = 0;
};

class IGPUIndexBuffer : public IGPUResource
{
public:
    virtual uint32_t getIndexCount() const = 0;
};

class IGPUVertexBuffer : public IGPUResource
{
public:
    virtual void updateData(const Vertex* data, uint32_t count) = 0;
    virtual uint32_t getVertexCount() const = 0;
};

class IGPUConstantBuffer : public IGPUResource
{
public:
    virtual void updateData(void* data) = 0;
    virtual uint32_t getBufferSize() const = 0;
};

class Shader : public Resource
{
public:
    Shader(const std::string& path) : Resource(path) {}
};

class VertexShader : public Shader
{
public:
    VertexShader(const std::string& path) : Shader(path) {}
};

class PixelShader : public Shader
{
public:
    PixelShader(const std::string& path) : Shader(path) {}
};

class Texture : public Resource
{
protected:
    int32_t mWidth;
    int32_t mHeight;
    int32_t mColorBit;
    IGPUTexture* mGPUResource;
public:
    Texture(const std::string& path, bool linear);
    Texture(const std::vector<std::string>& paths, bool linear);
    virtual ~Texture();

    int32_t getWidth() const { return mWidth; }
    int32_t getHeight() const { return mHeight; }

    IGPUTexture* getGPUResource() { return mGPUResource; }
};

class ColorF
{
public:
    float red, green, blue, alpha;
public:
    ColorF() : red(0), green(0), blue(0), alpha(1) {
    }
    ColorF(float r, float g, float b) : red(r), green(g), blue(b) {
    }
    ColorF(float r, float g, float b, float a) : red(r), green(g), blue(b), alpha(a) {
    }
};

class Material : public Resource
{
public:
    Texture* mDiffuseMap;
    ColorF mDiffuseColor;
    Texture* mNormalMap;
    Texture* mMetalnessMap;
    Texture* mRoughnessMap;
    ColorF mSpecularColor;
    Texture* mEmissionMap;
    bool mTwoSided;
public:
    Material(const std::string& name)
        : Resource(name),
            mDiffuseMap(nullptr),
            mNormalMap(nullptr),
            mMetalnessMap(nullptr),
            mRoughnessMap(nullptr),
            mEmissionMap(nullptr),
            mTwoSided(false) {
    }
    void setDiffuseMap(Texture* map) {
        mDiffuseMap = map;
    }
    Texture* getDiffuseMap() {
        return mDiffuseMap;
    }
    void setNormalMap(Texture* map) {
        mNormalMap = map;
    }
    Texture* getNormalMap() {
        return mNormalMap;
    }
    void setEmissionMap(Texture* map) {
        mEmissionMap = map;
    }
    Texture* getEmissionMap() {
        return mEmissionMap;
    }
    void setMetalnessMap(Texture* map) {
        mMetalnessMap = map;
    }
    void setRoughnessMap(Texture* map) {
        mRoughnessMap = map;
    }
    void setSpecularColor(const ColorF& color) {
        mSpecularColor = color;
    }
    const ColorF& getSpecularColor() const {
        return mSpecularColor;
    }
    void setTwoSided(bool enable) {
        mTwoSided = enable;
    }
    bool isTwoSided() const {
        return mTwoSided;
    }
};

class ResourceManager
{
protected:
    std::vector<Resource*> mResources;
    std::string mCommonShaderCodes;
public:
    ResourceManager();
    virtual ~ResourceManager();

    Material* createMaterial(const std::string& name);
    Mesh* createMesh(const std::string& name);
    SkeletonMesh* createSkeletonMesh(const std::string& name);

    IGPUShaderProgram* loadShaders(const char* vertex_file_path, const char* fragment_file_path, const char* geom_file_path = 0);
    Texture* loadTexture(const std::string& path, bool linear = false);
    Texture* loadCubeMapTexture(const std::vector<std::string>& paths, bool linear = false);
    Mesh* loadMesh(const std::string& path, const std::string& name, bool createCollisionMesh = false);
    SkeletonMesh* loadSkeletonMesh(const std::string& path, const std::string& name);
};

std::string loadFile(const char* file_path);

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoord;

    Vertex() {
    }

    Vertex(const glm::vec3& v) {
        position = v;
        normal = {0, 0, 0};
        tangent = {0, 0, 0};
        texCoord = {0, 0};
    }

    Vertex(const glm::vec3& p, float u, float v) {
        position = p;
        texCoord[0] = u;
        texCoord[1] = v;
    }

    Vertex(const glm::vec3& p, const glm::vec2& uv) {
        position = p;
        texCoord = uv;
    }

    Vertex(float x, float y, float z) {
        position[0] = x;
        position[1] = y;
        position[2] = z;
        normal = {0, 0, 0};
        tangent = {0, 0, 0};
        texCoord = {0, 0};
    }

    Vertex(float x, float y, float z, float u, float v) {
        position[0] = x;
        position[1] = y;
        position[2] = z;

        texCoord[0] = u;
        texCoord[1] = v;

        normal[0] = 0;
        normal[1] = 0;
        normal[2] = 0;
    }

    Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) {
        position[0] = x;
        position[1] = y;
        position[2] = z;

        texCoord[0] = u;
        texCoord[1] = v;

        normal[0] = nx;
        normal[1] = ny;
        normal[2] = nz;
    }

    inline bool operator == ( const Vertex& right ) const
    {
        return ( position == right.position &&
                texCoord == right.texCoord &&
                normal == right.normal );
    }
};

const int NUM_BONES_PER_VERTEX = 8;

struct AnimatedVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoord;
    float boneIDs[NUM_BONES_PER_VERTEX];
    float boneWeights[NUM_BONES_PER_VERTEX];

    AnimatedVertex() {
    }

    AnimatedVertex(const glm::vec3& v) {
        position = v;
    }

    AnimatedVertex(const glm::vec3& p, const glm::vec2& uv) {
        position = p;
        texCoord = uv;
    }

    inline bool operator == ( const AnimatedVertex& right ) const
    {
        return ( position == right.position &&
                texCoord == right.texCoord &&
                normal == right.normal );
    }
};

const Vertex quadVertices[] = {
    // positions        // texture Coords
    Vertex(-1.0f,    1.0f, 0.0f, 0.0f, 1.0f),
    Vertex(-1.0f,  -1.0f, 0.0f, 0.0f, 0.0f),
    Vertex(1.0f,    1.0f, 0.0f, 1.0f, 1.0f),
    Vertex(1.0f,  -1.0f, 0.0f, 1.0f, 0.0f),
};

const uint32_t quadIndices[] = {
    0, 1, 2,
    1, 3, 2
};

class QuadBufferIndexed
{
protected:
    IGPUVertexBuffer* mVertexBuffer;
    IGPUIndexBuffer* mIndexBuffer;
public:
    QuadBufferIndexed();
    bool create(Renderer* rnd);
    IGPUVertexBuffer* getVertexBuffer() {
        return mVertexBuffer;
    }
    IGPUIndexBuffer* getIndexBuffer() {
        return mIndexBuffer;
    }

    uint32_t getIndexCount() const {
        return mIndexBuffer->getIndexCount();
    }
};

enum class TextureFormat {
    RGBA8,
    SRGBA8,
    RGBA32_FLOAT,
    RGBA16_FLOAT,
    R32_FLOAT,
};

enum class TextureFilterType {
    PointFilter,
    LinearFilter,
};

enum FRAME_BUFFER_FLAG {
    FRAME_BUFFER_FLAG_COLOR = 1 << 0, // 1
    FRAME_BUFFER_FLAG_DEPTH_STENCIL = 1 << 1, // 2
    FRAME_BUFFER_FLAG_SHADOW = 1 << 2, // 4
    FRAME_BUFFER_FLAG_SHADOW_CUBE = 1 << 3, // 8
    FRAME_BUFFER_FLAG_DEPTH = 1 << 4, // 16
    Flag6 = 1 << 5, // 32
    Flag7 = 1 << 6, // 64
    Flag8 = 1 << 7  //128
};

struct RenderTargetDesc {
    TextureFilterType FilterType;
    TextureFormat Format;
};

struct FrameBufferDesc {
    uint32_t Width, Height;
    int Flags;
    TextureFilterType FilterType;
    int NumRenderTarget;
    std::vector<RenderTargetDesc> RenderTargetDescList;
};

enum CBufferBindType {
    CBBT_PS,
    CBBT_VS,
    CBBT_GS
};

class Renderer
{
protected:
    std::vector<QuadBufferIndexed*> mQuadBufferIndexedList;
public:
    Renderer() {}
    virtual ~Renderer();

    virtual bool init() = 0;

    virtual void swapBuffers() = 0;

    virtual IGPUTexture* createGPUTexture(int width, int height, void* data, TextureFormat format) = 0;
    virtual IGPUTexture* createGPUCubeMapTexture(int width, int height, std::vector<void*> dataList, TextureFormat format) = 0;

    virtual IGPUVertexBuffer* createGPUVertexBuffer(const Vertex* data, uint32_t count) = 0;
    virtual IGPUVertexBuffer* createGPUAnimatedVertexBuffer(const AnimatedVertex* data, uint32_t count) = 0;
    virtual IGPUIndexBuffer* createGPUIndexBuffer(const uint32_t* data, uint32_t count) = 0;

    virtual QuadBufferIndexed* createQuadBufferIndexed();

    virtual IGPUConstantBuffer* createGPUConstantBuffer(uint32_t sizeinBytes) = 0;

    virtual IGPUResource* createVertexShader(const std::string& code) = 0;
    virtual IGPUResource* createPixelShader(const std::string& code) = 0;
    virtual IGPUResource* createGeometryShader(const std::string& code) = 0;
    virtual IGPUShaderProgram* createGPUProgram(IGPUResource* vs, IGPUResource* ps, IGPUResource* gs) = 0;

    virtual FrameBuffer* createFrameBufferObject(const FrameBufferDesc& desc) = 0;

    virtual void bindFrameBuffer(FrameBuffer* fb, const ColorF& color) = 0;

    virtual void bindGPUTexture(IGPUTexture* tex, int index) = 0;

    virtual void bindConstantBuffer(IGPUConstantBuffer* buffer, enum CBufferBindType type, uint32_t index) { assert(0); }

    virtual void bindQuadBuffer(QuadBufferIndexed* qb);

    virtual void bindResource(IGPUResource* r) = 0;
    virtual void unbindResource(IGPUResource* r) = 0;

    virtual void setDepthTest(bool enable) = 0;
    virtual void setViewport(float left, float top, float width, float height) = 0;

    virtual void draw(uint32_t numTriangle) = 0;
    virtual void drawInstanced(uint32_t numTriangle, uint32_t numInstance) = 0;
    virtual void drawNonIndexed(uint32_t numVertices) = 0;
};

class FrameBuffer : public IGPUResource
{
public:
    FrameBuffer() { }
    virtual ~FrameBuffer() { }
    virtual const FrameBufferDesc& getDescription() const = 0;
    virtual uint64_t getRenderingId() const  = 0;
    virtual IGPUTexture* getDepthAttachmentId() const = 0;
    virtual IGPUTexture* getColorAttachmentId(uint32_t index) const = 0;

    virtual uint64_t getResourceId() const = 0;
    virtual GPUResourceType getType() const { return GRT_FRAMEBUFFER; }
};

class IGPUShaderProgram : public IGPUResource
{
public:
    IGPUShaderProgram() { }
    virtual ~IGPUShaderProgram() { }

    virtual GPUResourceType getType() const { return GRT_SHADER_PROGRAM; }
};







