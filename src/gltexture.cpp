#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "glsystem.h"
#include <glad\glad.h>

GLuint getGLTextureFormat(TextureFormat format) {
    GLuint glFormat = 0;
    switch(format) {
    case TextureFormat::RGBA8:
        glFormat = GL_RGBA8;
        break;
    case TextureFormat::SRGBA8:
        glFormat = GL_SRGB8_ALPHA8;
        break;
    case TextureFormat::RGBA16_FLOAT:
        glFormat = GL_RGBA16F;
        break;
    case TextureFormat::RGBA32_FLOAT:
        glFormat = GL_RGBA32F;
        break;
    case TextureFormat::R32_FLOAT:
        glFormat = GL_R32F;
        break;
    default:
        assert(0);
    }
    return glFormat;
}

GLTexture::GLTexture(const GLCubeMapTextureDesc& desc)
    : mWidth(desc.Width), mHeight(desc.Height), mCubeMap(true)
{
    assert(desc.DataList.size() == 6);

    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureId);

    for(int i = 0;i < 6;i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, desc.InternalFormat, desc.Width, desc.Height, 0,
              desc.Format, desc.DataType, desc.DataList[i]);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, desc.WrapType);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,  desc.WrapType);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, desc.WrapType);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

GLTexture::GLTexture(const GLTextureDesc& desc, bool cubemap)
    : mWidth(desc.Width), mHeight(desc.Height), mCubeMap(cubemap)
{
    glGenTextures(1, &mTextureId);

    if (cubemap) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureId);

        for(int i = 0;i < 6;i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, desc.InternalFormat, desc.Width, desc.Height, 0,
                  desc.Format, desc.DataType, desc.Data);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, desc.WrapType);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,  desc.WrapType);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    } else {
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, desc.WrapType);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  desc.WrapType);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (desc.DoMipMap) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            // Also enable the mighty ANISOTROPY FILTER!

            //GLfloat max_anisotropy; /* don't exceed this value...*/
            //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
            //printf("OpenGL Max Anisotropy Supported: %f\n", max_anisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, desc.InternalFormat, desc.Width, desc.Height, 0,
                  desc.Format, desc.DataType, desc.Data);

        if (desc.DoMipMap) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

GLTexture::~GLTexture() {
    glDeleteTextures(1, &mTextureId);
}

