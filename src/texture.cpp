#include "stdafx.h"
#include "engine.h"
#include "renderer.h"

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>


unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 4);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

Texture::Texture(const std::string& path, bool linear)
    : Resource(path), mGPUResource(nullptr) {

    printf("loading texture '%s'...", path.c_str());
    unsigned char* data = stbi_load(path.c_str(), &mWidth, &mHeight, &mColorBit, STBI_rgb_alpha);

    if (data) {
        printf(" Success %d x %d, %d-Bit\n", mWidth, mHeight, mColorBit * 8);
        Renderer* rnd = Engine::get()->getRenderingSystem();
        mGPUResource = rnd->createGPUTexture(mWidth, mHeight, data, linear ? TextureFormat::RGBA8 : TextureFormat::SRGBA8);
        // we don't need to keep the data in ram, it's a GPU resource
        stbi_image_free(data);
    } else {
        printf("\nfailed to load texture '%s'\n", path.c_str());
    }
}

Texture::Texture(const std::vector<std::string>& paths, bool linear)
    : Resource("cubemap"), mGPUResource(nullptr) {

    printf("loading cube map texture...\n");

    std::vector<void*> dataList;
    dataList.reserve(paths.size());

    for (unsigned int i = 0; i < paths.size(); i++) {
        unsigned char* data = stbi_load(paths[i].c_str(), &mWidth, &mHeight, &mColorBit, STBI_rgb_alpha);
        if (data) {
            dataList.push_back(data);
        }
        else {
            std::cout << "Failed to load cube map tecture at path: " << paths[i] << std::endl;
        }
    }

    Renderer* rnd = Engine::get()->getRenderingSystem();
    mGPUResource = rnd->createGPUCubeMapTexture(mWidth, mHeight, dataList, linear ? TextureFormat::RGBA8 : TextureFormat::SRGBA8);

    for (unsigned int i = 0; i < dataList.size(); i++) {
        stbi_image_free(dataList[i]);
    }
}

Texture::~Texture() {
}




