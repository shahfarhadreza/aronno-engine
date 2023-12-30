#pragma once

#include "textparser.h"

struct ObjMaterial {
    std::string name;
    ColorF diffuseColor;
    ColorF specularColor;
    std::string texturePath = "";
    std::string normalPath = "";
    std::string roughPath = "";
    std::string metalPath = "";
    std::string emissivePath = "";
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
};

using VertexCache = std::unordered_multimap<uint32_t, uint32_t>;


const uint32_t reserve_rate = 100;

class ObjLoader {
protected:
    std::map<std::string, ObjMaterial*> mats;
    Renderer* mRenderingContext;
    VertexCache vertices_map;
    uint32_t vertices_map_reserved;
public:
    ObjLoader(Renderer* rc) : mRenderingContext(rc) {
        vertices_map_reserved = reserve_rate;
    }
    virtual ~ObjLoader() {
        for(auto it = mats.begin();it != mats.end();++it) {
            ObjMaterial* omat = it->second;
            delete omat;
        }
    }
    bool loadMaterialLibrary(const std::string& path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            return false;
        }
        std::string curline;

        ObjMaterial* currentMat = nullptr;

        while (std::getline(file, curline)) {
            std::string first = obj_firstToken(curline);
            if (first == "newmtl") {
                std::string matName = obj_tail(curline);
                if (mats.find(matName) == mats.end()) {
                    currentMat = new ObjMaterial;
                    currentMat->name = matName;

                    mats[matName] = currentMat;
                } else {
                    currentMat = mats[matName];
                }
            }

            if (first == "map_Kd") {
                assert(currentMat);
                currentMat->texturePath = obj_tail(curline);
            }

            if (first == "map_bump") {
                assert(currentMat);
                currentMat->normalPath = obj_tail(curline);
            }

            if (first == "map_Ks") {
                assert(currentMat);
                currentMat->roughPath = obj_tail(curline);
            }

            if (first == "map_refl") {
                assert(currentMat);
                currentMat->metalPath = obj_tail(curline);
            }

            if (first == "map_Ke") {
                assert(currentMat);
                currentMat->emissivePath = obj_tail(curline);
            }

            if (first == "Kd") {
                assert(currentMat);
                std::vector<std::string> scolor;
                obj_split(obj_tail(curline), scolor, " ");

                currentMat->diffuseColor.red = std::stof(scolor[0]);
                currentMat->diffuseColor.green = std::stof(scolor[1]);
                currentMat->diffuseColor.blue = std::stof(scolor[2]);
            }

            if (first == "Ks") {
                assert(currentMat);
                std::vector<std::string> scolor;
                obj_split(obj_tail(curline), scolor, " ");

                currentMat->specularColor.red = std::stof(scolor[0]);
                currentMat->specularColor.green = std::stof(scolor[1]);
                currentMat->specularColor.blue = std::stof(scolor[2]);
            }
        }
        return true;
    }

    uint32_t add_vertex(uint32_t hash, const Vertex* pVertex, std::vector<Vertex>& vertices) {
        auto f = vertices_map.equal_range(hash);

        for (auto it = f.first; it != f.second; ++it) {
            if (it->second >= vertices.size()) {
                continue;
            }
            assert(it->second < vertices.size());
            const Vertex& tv = vertices[it->second];

            if (0 == memcmp(pVertex, &tv, sizeof(Vertex))) {
                return it->second;
            }
        }

        auto index = static_cast<uint32_t>(vertices.size());
        vertices.emplace_back(*pVertex);

        VertexCache::value_type entry(hash, index);
        vertices_map.insert(entry);
        return index;
    };

    bool loadFromFile(const std::string& modelpath, Mesh* mesh, bool createCollisionMesh) {
        std::ifstream file(modelpath);

        if (!file.is_open()) {
            return false;
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> tcoords;
        std::vector<glm::vec3> normals;

        vertices_map.reserve(vertices_map_reserved);

        positions.reserve(reserve_rate);
        tcoords.reserve(reserve_rate);
        normals.reserve(reserve_rate);

        ObjMaterial* currentMat = nullptr;
        std::string matLibrary = "";

        AABB boundingBox;

        printf("parsing obj (%s)...\n", modelpath.c_str());
        std::string curline;
        while (std::getline(file, curline)) {
            if (obj_firstToken(curline) == "mtllib") {
                matLibrary = obj_tail(curline);
            }
            // Generate a Vertex Position
            if (obj_firstToken(curline) == "v") {
                std::vector<std::string> spos;
                glm::vec3 vpos;
                obj_split(obj_tail(curline), spos, " ");

                vpos.x = std::stof(spos[0]);
                vpos.y = std::stof(spos[1]);
                vpos.z = std::stof(spos[2]);

                positions.push_back(vpos);
                boundingBox.extend(vpos);
            }
            // Generate a Vertex Texture Coordinate
            if (obj_firstToken(curline) == "vt") {
                std::vector<std::string> stex;
                glm::vec2 vtex;
                obj_split(obj_tail(curline), stex, " ");

                vtex.x = std::stof(stex[0]);
                vtex.y = 1 - std::stof(stex[1]);

                tcoords.push_back(vtex);
            }
            // Generate a Vertex Normal;
            if (obj_firstToken(curline) == "vn") {
                std::vector<std::string> snor;
                glm::vec3 vnor;
                obj_split(obj_tail(curline), snor, " ");

                vnor.x = std::stof(snor[0]);
                vnor.y = std::stof(snor[1]);
                vnor.z = std::stof(snor[2]);

                normals.push_back(vnor);
            }

            if (obj_firstToken(curline) == "usemtl") {
                std::string matName = obj_tail(curline);
                /*
                if (!currentMat) {
                    currentMat = new ObjMaterial;
                    currentMat->name = matName;
                    mats[matName] = currentMat;
                }
                */
                if (mats.find(matName) == mats.end()) {
                    currentMat = new ObjMaterial;
                    currentMat->name = matName;
                    mats[matName] = currentMat;
                } else {
                    currentMat = mats[matName];
                }
            }

            if (obj_firstToken(curline) == "f") {
                std::vector<std::string> sface, svert;
                obj_split(obj_tail(curline), sface, " ");

                assert(currentMat);

                // For every given vertex do this
                assert(sface.size() == 3);
                for (size_t i = 0; i < sface.size(); i++) {
                    obj_split(sface[i], svert, "/");

                    Vertex v;

                    uint32_t pos_idx = std::stoi(svert[0]) - 1;
                    uint32_t t_idx = std::stoi(svert[1]) - 1;
                    uint32_t norm_idx = std::stoi(svert[2]) - 1;

                    assert(pos_idx < positions.size());
                    assert(t_idx < tcoords.size());
                    assert(norm_idx < normals.size());

                    v.position = positions[pos_idx];
                    v.texCoord = tcoords[t_idx];
                    v.normal = normals[norm_idx];

                    v.tangent = glm::vec3(0, 0, 0);
                    //v.bitangent = glm::vec3(0, 0, 0);

                    auto index = add_vertex(pos_idx, &v, currentMat->vertices);
                    currentMat->indices.push_back(index);
                }
            }
        }

        printf("obj loaded successfully\n");

        if (matLibrary != "") {
            printf("loading material library...\n");
            loadMaterialLibrary(matLibrary);
        }

        mesh->setBoundingBox(boundingBox);

        ResourceManager* mgr = Engine::get()->getResourceManager();

        btTriangleMesh* bulletMesh = mesh->getCollisionMesh();

        // Create Sub Meshes per material
        printf("creating sub meshes per material...\n");
        for(auto it = mats.begin();it != mats.end();++it) {

            ObjMaterial* omat = it->second;
            Material* mat = mgr->createMaterial(omat->name);

            size_t found = omat->name.find("twoside");
            if (found != std::string::npos) {
                mat->setTwoSided(true);
            }

            size_t nocollision = omat->name.find("nocollision");

            if (!omat->texturePath.empty()) {
                printf("loading diffuse texture %s\n", omat->texturePath.c_str());
                Texture* tex = mgr->loadTexture(omat->texturePath);
                mat->setDiffuseMap(tex);
            }

            if (!omat->normalPath.empty()) {
                Texture* tex = mgr->loadTexture(omat->normalPath, true);
                mat->setNormalMap(tex);

                // if we have a normal map then we have calculate tangent stuffs...
                for(size_t i = 0;i < omat->indices.size(); i += 3) {
                    const uint32_t idx0 = omat->indices[i];
                    const uint32_t idx1 = omat->indices[i + 1];
                    const uint32_t idx2 = omat->indices[i + 2];

                    Vertex& v0 = omat->vertices[idx0];
                    Vertex& v1 = omat->vertices[idx1];
                    Vertex& v2 = omat->vertices[idx2];

                    glm::vec3 deltaPos1 = v1.position - v0.position;
                    glm::vec3 deltaPos2 = v2.position - v0.position;

                    glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
                    glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

                    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                    glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;

                    //glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

                    //tangent = glm::normalize(tangent);
                    //bitangent = glm::normalize(bitangent);

                    v0.tangent = tangent;
                    v1.tangent = tangent;
                    v2.tangent = tangent;

                    //v0.bitangent = bitangent;
                    //v1.bitangent = bitangent;
                    //v2.bitangent = bitangent;
                }
            }
            mat->setSpecularColor(omat->specularColor);

            if (!omat->emissivePath.empty()) {
                printf("loading emissiion texture %s\n", omat->emissivePath.c_str());
                Texture* tex = mgr->loadTexture(omat->emissivePath);
                mat->setEmissionMap(tex);
            }

            if (!omat->metalPath.empty()) {
                printf("loading metalness texture %s\n", omat->metalPath.c_str());
                Texture* tex = mgr->loadTexture(omat->metalPath, true);
                mat->setMetalnessMap(tex);
            }

            if (!omat->roughPath.empty()) {
                printf("loading roughness texture %s\n", omat->roughPath.c_str());
                Texture* tex = mgr->loadTexture(omat->roughPath, true);
                mat->setRoughnessMap(tex);
            }

            AABB bbSubMesh;

            for(size_t i = 0;i < omat->vertices.size(); i++) {
                Vertex& v = omat->vertices[i];
                bbSubMesh.extend(v.position);
            }

            auto vb = mRenderingContext->createGPUVertexBuffer(&omat->vertices[0], omat->vertices.size());
            auto ib = mRenderingContext->createGPUIndexBuffer(&omat->indices[0], omat->indices.size());

           // printf("texture %s\n", mat->texturePath.c_str());
            SubMesh* sm = mesh->createSubMesh(vb, ib);
            sm->setMaterial(mat);
            sm->setLocalBoundingBox(bbSubMesh);

            // Build a bullet physics mesh for collision
            if (nocollision == std::string::npos) {
                if (createCollisionMesh) {
                    for(size_t i = 0;i < omat->indices.size(); i += 3) {
                        const uint32_t idx0 = omat->indices[i];
                        const uint32_t idx1 = omat->indices[i + 1];
                        const uint32_t idx2 = omat->indices[i + 2];

                        Vertex& v0 = omat->vertices[idx0];
                        Vertex& v1 = omat->vertices[idx1];
                        Vertex& v2 = omat->vertices[idx2];

                        btVector3 p0(v0.position.x, v0.position.y, v0.position.z);
                        btVector3 p1(v1.position.x, v1.position.y, v1.position.z);
                        btVector3 p2(v2.position.x, v2.position.y, v2.position.z);

                        bulletMesh->addTriangle(p0, p1, p2);
                    }
                }
            }
        }

        return true;
    }
};


