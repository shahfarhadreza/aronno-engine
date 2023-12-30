#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "glsystem.h"
#include <glad\glad.h>

OpenGLRenderer::OpenGLRenderer(GLFWwindow* windowHandle) : mWindowHandle(windowHandle) {

}

bool OpenGLRenderer::init() {
    glfwMakeContextCurrent(mWindowHandle);

	// GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	setDepthTest(true);

	glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 0);
	glLineWidth(2);
    return true;
}

OpenGLRenderer::~OpenGLRenderer() {
    for(auto it = mResources.begin(); it!= mResources.end();++it) {
        delete *it;
    }
    mResources.clear();
}

void OpenGLRenderer::swapBuffers() {
    glfwSwapBuffers(mWindowHandle);
}

IGPUTexture* OpenGLRenderer::createGPUTexture(int width, int height, void* data, TextureFormat format) {
    GLTextureDesc tdesc = {
        .Width = width,
        .Height = height,
        .Data = data,
        .DoMipMap = true,
        .InternalFormat = getGLTextureFormat(format),
        .WrapType = GL_REPEAT,
        .Format = GL_RGBA,
        .DataType = GL_UNSIGNED_BYTE,
    };
    IGPUTexture* r = new GLTexture(tdesc, false);
    mResources.push_back(r);
    return r;
}

IGPUTexture* OpenGLRenderer::createGPUCubeMapTexture(int width, int height, std::vector<void*> dataList, TextureFormat format) {
    GLCubeMapTextureDesc tdesc = {
        .Width = width,
        .Height = height,
        .DataList = dataList,
        .DoMipMap = true,
        .InternalFormat = getGLTextureFormat(format),
        .WrapType = GL_CLAMP_TO_EDGE,
        .Format = GL_RGBA,
        .DataType = GL_UNSIGNED_BYTE,
    };
    IGPUTexture* r = new GLTexture(tdesc);
    mResources.push_back(r);
    return r;
}

IGPUVertexBuffer* OpenGLRenderer::createGPUVertexBuffer(const Vertex* data, uint32_t count) {
    IGPUVertexBuffer* r = new GLVertexBuffer(data, count);
    mResources.push_back(r);
    return r;
}

IGPUVertexBuffer* OpenGLRenderer::createGPUAnimatedVertexBuffer(const AnimatedVertex* data, uint32_t count) {
    IGPUVertexBuffer* r = new GLVertexBuffer(data, count);
    mResources.push_back(r);
    return r;
}

IGPUIndexBuffer* OpenGLRenderer::createGPUIndexBuffer(const uint32_t* data, uint32_t count) {
    IGPUIndexBuffer* r = new GLIndexBuffer(data, count);
    mResources.push_back(r);
    return r;
}

IGPUShaderProgram* OpenGLRenderer::createGPUProgram(IGPUResource* vs, IGPUResource* ps, IGPUResource* gs) {
    IGPUShaderProgram* r = new GLProgram(dynamic_cast<GLShader*>(vs), dynamic_cast<GLShader*>(ps), dynamic_cast<GLShader*>(gs));
    mResources.push_back(r);
    return r;
}

IGPUResource* OpenGLRenderer::createVertexShader(const std::string& code) {
    IGPUResource* r = new GLShader(GL_VERTEX_SHADER, code.c_str());
    mResources.push_back(r);
    return r;
}
IGPUResource* OpenGLRenderer::createPixelShader(const std::string& code) {
    IGPUResource* r = new GLShader(GL_FRAGMENT_SHADER, code.c_str());
    mResources.push_back(r);
    return r;
}

IGPUResource* OpenGLRenderer::createGeometryShader(const std::string& code) {
    IGPUResource* r = new GLShader(GL_GEOMETRY_SHADER, code.c_str());
    mResources.push_back(r);
    return r;
}

FrameBuffer* OpenGLRenderer::createFrameBufferObject(const FrameBufferDesc& desc) {
    FrameBuffer* r = new GLFrameBuffer(desc);
    mResources.push_back(r);
    return r;
}

void OpenGLRenderer::bindFrameBuffer(FrameBuffer* fb, const ColorF& color) {
    if (!fb) {
        int width, height;
        glfwGetWindowSize(mWindowHandle, &width, &height);
        setViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        const FrameBufferDesc& desc = fb->getDescription();
        setViewport(0, 0, desc.Width, desc.Height);
        glBindFramebuffer(GL_FRAMEBUFFER, fb->getRenderingId());
    }
    glClearColor(color.red, color.green, color.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

IGPUConstantBuffer* OpenGLRenderer::createGPUConstantBuffer(uint32_t sizeinBytes) {
    IGPUConstantBuffer* r = new GLConstantBuffer(sizeinBytes);
    mResources.push_back(r);
    return r;
}

void OpenGLRenderer::bindConstantBuffer(IGPUConstantBuffer* buffer, enum CBufferBindType type, uint32_t index) {
    glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer->getResourceId());
}

void OpenGLRenderer::bindGPUTexture(IGPUTexture* tex, int index) {
    if (tex == nullptr) {
        return;
    }
    switch(index) {
    case 0:
        glActiveTexture(GL_TEXTURE0);
        break;
    case 1:
        glActiveTexture(GL_TEXTURE1);
        break;
    case 2:
        glActiveTexture(GL_TEXTURE2);
        break;
    case 3:
        glActiveTexture(GL_TEXTURE3);
        break;
    case 4:
        glActiveTexture(GL_TEXTURE4);
        break;
    case 5:
        glActiveTexture(GL_TEXTURE5);
        break;
    case 6:
        glActiveTexture(GL_TEXTURE6);
        break;
    case 7:
        glActiveTexture(GL_TEXTURE7);
        break;
    case 8:
        glActiveTexture(GL_TEXTURE8);
        break;
    case 9:
        glActiveTexture(GL_TEXTURE9);
        break;
    case 10:
        glActiveTexture(GL_TEXTURE10);
        break;
    case 11:
        glActiveTexture(GL_TEXTURE11);
        break;
    case 12:
        glActiveTexture(GL_TEXTURE12);
        break;
    case 13:
        glActiveTexture(GL_TEXTURE13);
        break;
    case 14:
        glActiveTexture(GL_TEXTURE14);
        break;
    case 15:
        glActiveTexture(GL_TEXTURE15);
        break;
    case 16:
        glActiveTexture(GL_TEXTURE16);
        break;
    case 17:
        glActiveTexture(GL_TEXTURE17);
        break;
    case 18:
        glActiveTexture(GL_TEXTURE18);
        break;
    case 19:
        glActiveTexture(GL_TEXTURE19);
        break;
    case 20:
        glActiveTexture(GL_TEXTURE20);
        break;
    case 21:
        glActiveTexture(GL_TEXTURE21);
        break;
    case 22:
        glActiveTexture(GL_TEXTURE22);
        break;
    case 23:
        glActiveTexture(GL_TEXTURE23);
        break;
    case 24:
        glActiveTexture(GL_TEXTURE24);
        break;
    case 25:
        glActiveTexture(GL_TEXTURE25);
        break;
    case 26:
        glActiveTexture(GL_TEXTURE26);
        break;
    default:
        assert(0);
    }
    if (tex->isCubeMap()) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex->getResourceId());
    } else {
        glBindTexture(GL_TEXTURE_2D, tex->getResourceId());
    }
}

void OpenGLRenderer::bindResource(IGPUResource* r) {
    if (r->getType() == GRT_TEXTURE) {
        assert(0);
    } else if (r->getType() == GRT_VERTEX_BUFFER) {
        GLVertexBuffer* vb = dynamic_cast<GLVertexBuffer*>(r);
        glBindBuffer(GL_ARRAY_BUFFER, r->getResourceId());
        glBindVertexArray(vb->getVertexArrayObject());
    } else if (r->getType() == GRT_INDEX_BUFFER) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->getResourceId());
    } else if (r->getType() == GRT_SHADER_PROGRAM) {
        glUseProgram(r->getResourceId());
    }
}

void OpenGLRenderer::unbindResource(IGPUResource* r) {
    if (r->getType() == GRT_TEXTURE) {
        glBindTexture(GL_TEXTURE_2D, 0);
    } else if (r->getType() == GRT_VERTEX_BUFFER) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    } else if (r->getType() == GRT_INDEX_BUFFER) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else if (r->getType() == GRT_SHADER_PROGRAM) {
        glUseProgram(0);
    }
}

void OpenGLRenderer::setViewport(float left, float top, float width, float height) {
    glViewport(left, top, width, height);
}

void OpenGLRenderer::setDepthTest(bool enable) {
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void OpenGLRenderer::draw(uint32_t numTriangle) {
    glDrawElements(GL_TRIANGLES, numTriangle, GL_UNSIGNED_INT, 0);
}

void OpenGLRenderer::drawInstanced(uint32_t numTriangle, uint32_t numInstance) {
    glDrawElementsInstanced(GL_TRIANGLES, numTriangle, GL_UNSIGNED_INT, 0, numInstance);
}

void OpenGLRenderer::drawNonIndexed(uint32_t numVertices) {
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}



