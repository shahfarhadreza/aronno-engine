#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "glsystem.h"


GLFrameBuffer::GLFrameBuffer(const FrameBufferDesc& desc)
    : mBufferId(-1), mDesc(desc), mDepthAttachment(nullptr) {
    glGenFramebuffers(1, &mBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, mBufferId);

    // Color attachment
    if (desc.Flags & FRAME_BUFFER_FLAG_COLOR) {
        mColorAttachmentList.resize(desc.NumRenderTarget);

        for(int i = 0; i < desc.NumRenderTarget;++i) {

            const RenderTargetDesc& rtDesc = desc.RenderTargetDescList[i];

            GLTextureDesc tdesc = {
                .Width = desc.Width,
                .Height = desc.Height,
                .Data = nullptr,
                .DoMipMap = false,
                .InternalFormat = getGLTextureFormat(rtDesc.Format),
                .WrapType = GL_CLAMP_TO_EDGE,
                .Format = GL_RGBA,
                .DataType = GL_UNSIGNED_BYTE,
            };

            mColorAttachmentList[i] = new GLTexture(tdesc, false);
            GLuint textureId = mColorAttachmentList[i]->getResourceId();

            GLuint attachmentIdx = GL_COLOR_ATTACHMENT0;

            switch(i) {
            case 0:
                attachmentIdx = GL_COLOR_ATTACHMENT0;
                break;
            case 1:
                attachmentIdx = GL_COLOR_ATTACHMENT1;
                break;
            case 2:
                attachmentIdx = GL_COLOR_ATTACHMENT2;
                break;
            default:
                assert(0);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentIdx, GL_TEXTURE_2D, textureId, 0);
        }

        GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, DrawBuffers);
        /*
        glGenTextures(1, &mColorAttachment);
        glBindTexture(GL_TEXTURE_2D, mColorAttachment);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     desc.Width, desc.Height, 0, GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttachment, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
        */
    }

    // Depth attachment
    if (desc.Flags & FRAME_BUFFER_FLAG_DEPTH) {

        GLTextureDesc tdesc = {
            .Width = desc.Width,
            .Height = desc.Height,
            .Data = nullptr,
            .DoMipMap = false,
            .InternalFormat = GL_DEPTH_COMPONENT32F,
            .WrapType = GL_CLAMP_TO_EDGE,
            .Format = GL_DEPTH_COMPONENT,
            .DataType = GL_FLOAT,
        };

        mDepthAttachment = new GLTexture(tdesc, false);

        glBindTexture(GL_TEXTURE_2D, mDepthAttachment->getResourceId());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, mDepthAttachment->getResourceId(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (desc.Flags & FRAME_BUFFER_FLAG_SHADOW) {

        GLTextureDesc tdesc = {
            .Width = desc.Width,
            .Height = desc.Height,
            .Data = nullptr,
            .DoMipMap = false,
            .InternalFormat = GL_DEPTH_COMPONENT32F,
            .WrapType = GL_CLAMP_TO_EDGE,
            .Format = GL_DEPTH_COMPONENT,
            .DataType = GL_FLOAT,
        };

        mDepthAttachment = new GLTexture(tdesc, false);

        glBindTexture(GL_TEXTURE_2D, mDepthAttachment->getResourceId());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, mDepthAttachment->getResourceId(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (desc.Flags & FRAME_BUFFER_FLAG_SHADOW_CUBE) {

        GLTextureDesc tdesc = {
            .Width = desc.Width,
            .Height = desc.Height,
            .Data = nullptr,
            .DoMipMap = false,
            .InternalFormat = GL_DEPTH_COMPONENT,
            .WrapType = GL_CLAMP_TO_EDGE,
            .Format = GL_DEPTH_COMPONENT,
            .DataType = GL_FLOAT,
        };

        mDepthAttachment = new GLTexture(tdesc, true);

        glBindTexture(GL_TEXTURE_2D, mDepthAttachment->getResourceId());
/*
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
*/

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthAttachment->getResourceId(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (desc.Flags & FRAME_BUFFER_FLAG_DEPTH_STENCIL) {
        GLTextureDesc tdesc = {
            .Width = desc.Width,
            .Height = desc.Height,
            .Data = nullptr,
            .DoMipMap = false,
            .InternalFormat = GL_DEPTH24_STENCIL8,
            .WrapType = GL_CLAMP_TO_EDGE,
            .Format = GL_DEPTH_STENCIL,
            .DataType = GL_UNSIGNED_INT_24_8,
        };

        mDepthAttachment = new GLTexture(tdesc, false);
        /*
        glGenTextures(1, &mDepthAttachment);
        glBindTexture(GL_TEXTURE_2D, mDepthAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
                     desc.Width, desc.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        */

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, mDepthAttachment->getResourceId(), 0);

        //glBindTexture(GL_TEXTURE_2D, 0);
    }
    // unbind all
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLFrameBuffer::~GLFrameBuffer() {
    if (mDepthAttachment) {
        delete mDepthAttachment;
    }
    for(auto it = mColorAttachmentList.begin(); it!= mColorAttachmentList.end();++it) {
        delete *it;
    }
    mColorAttachmentList.clear();
    //if (mColorAttachment) {
        //glDeleteTextures(1, &mColorAttachment);
    //}
    glDeleteFramebuffers(1, &mBufferId);
}

