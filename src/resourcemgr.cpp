#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "mesh.h"
#include "meshloader.h"

#include <filesystem>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}

static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
{
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

void buildSkeleton(Skeleton* skeleton, const aiScene* scene, const aiNode* pNode, Bone* parentBone);
void readKeyFrames(Skeleton* skeleton, const aiScene* scene);

std::string loadFile(const char * file_path) {
    std::cout << "Loading file " << file_path << " ..." << std::endl;
    std::string VertexShaderCode;
	std::ifstream VertexShaderStream(file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", file_path);
		getchar();
	}
	return VertexShaderCode;
}

ResourceManager::ResourceManager() {
    mCommonShaderCodes = loadFile("shaders/glsl/common.glsl");
}

ResourceManager::~ResourceManager() {
    for(auto it = mResources.begin(); it!= mResources.end();++it) {
        Resource* r = *it;
        delete r;
    }
    mResources.clear();
}

IGPUShaderProgram* ResourceManager::loadShaders(const char* vertex_file_path, const char* fragment_file_path, const char* geom_file_path ) {
	// Read the Vertex Shader code from the file
	std::string VertexShaderCode = loadFile(vertex_file_path);
	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode = loadFile(fragment_file_path);

	VertexShaderCode = mCommonShaderCodes + "\r\n\r\n" + VertexShaderCode;
	FragmentShaderCode = mCommonShaderCodes + "\r\n\r\n" + FragmentShaderCode;

	Renderer* rs = Engine::get()->getRenderingSystem();

	IGPUResource* vs = rs->createVertexShader(VertexShaderCode);
	IGPUResource* ps = rs->createPixelShader(FragmentShaderCode);
	IGPUResource* gs = nullptr;

	if (geom_file_path != 0) {
        std::string GeomShaderCode = loadFile(geom_file_path);
        gs = rs->createGeometryShader(GeomShaderCode);
	}

    IGPUShaderProgram* p = rs->createGPUProgram(vs, ps, gs);
    return p;
}

Material* ResourceManager::createMaterial(const std::string& name) {
    Material* tex = new Material(name);
    mResources.push_back(tex);
    return tex;
}

Texture* ResourceManager::loadCubeMapTexture(const std::vector<std::string>& paths, bool linear) {
    Texture* tex = new Texture(paths, linear);
    mResources.push_back(tex);
    return tex;
}

Texture* ResourceManager::loadTexture(const std::string& path, bool linear) {
    Texture* tex = new Texture(path, linear);
    mResources.push_back(tex);
    return tex;
}

Mesh* ResourceManager::createMesh(const std::string& name) {
    Mesh* mesh = new Mesh(name);
    mResources.push_back(mesh);
    return mesh;
}

SkeletonMesh* ResourceManager::createSkeletonMesh(const std::string& name) {
    SkeletonMesh* mesh = new SkeletonMesh(name);
    mResources.push_back(mesh);
    return mesh;
}

Mesh* ResourceManager::loadMesh(const std::string& path, const std::string& name, bool createCollisionMesh) {
    Renderer* rend = Engine::get()->getRenderingSystem();
    ObjLoader* loader = new ObjLoader(rend);
    Mesh* mesh = this->createMesh(name);
    if (!loader->loadFromFile(path, mesh, createCollisionMesh)) {
        std::cout << "ERROR: Failed to load mesh: " << path << std::endl;
        delete mesh;
        delete loader;
        return nullptr;
    }
    delete loader;
    return mesh;
}

struct NodeInfo {
    const aiNode* pNode = nullptr;
    bool isRequired = false;
};

std::map<std::string, NodeInfo> m_requiredNodeMap;

void InitializeRequiredNodeMap(const aiNode* pNode) {
    std::string NodeName(pNode->mName.C_Str());

    NodeInfo info;
    info.pNode = pNode;
    info.isRequired = false;

    m_requiredNodeMap[NodeName] = info;

    for (unsigned int i = 0 ; i < pNode->mNumChildren ; i++) {
        InitializeRequiredNodeMap(pNode->mChildren[i]);
    }
}

void MarkRequiredNodesForBone(const aiBone* pBone) {
    std::string NodeName(pBone->mName.C_Str());

    const aiNode* pParent = NULL;

    do {
        std::map<std::string,NodeInfo>::iterator it = m_requiredNodeMap.find(NodeName);

        if (it == m_requiredNodeMap.end()) {
            printf("Cannot find bone %s in the hierarchy\n", NodeName.c_str());
            assert(0);
        }

        it->second.isRequired = true;

        pParent = it->second.pNode->mParent;

        if (pParent) {
            NodeName = std::string(pParent->mName.C_Str());
        }

    } while (pParent);
}

std::map<std::string, aiNode*> meshNodeIndexed;

void buildMeshTransforms(const aiScene* scene, aiNode* pNode) {

    for (unsigned int i = 0; i < pNode->mNumMeshes;i++) {
        uint32_t meshIndex = pNode->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[meshIndex];
        std::string meshName(mesh->mName.data);
        meshNodeIndexed[meshName] = pNode;
    }
    for (uint32_t i = 0 ; i < pNode->mNumChildren ; i++) {
        buildMeshTransforms(scene, pNode->mChildren[i]);
    }
}

SkeletonMesh* ResourceManager::loadSkeletonMesh(const std::string& path, const std::string& name) {
    // Create an instance of the Importer class
    Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.

    //importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs);

    SkeletonMesh* mAnimatedMesh = nullptr;

    // If the import failed, report it
    if (nullptr == scene) {
        //DoTheErrorLogging( importer.GetErrorString());
        //return false;
        std::cout << "ERROR: Failed to import asset using AssimpLib: " << importer.GetErrorString() << std::endl;
    } else {
        std::cout << "Creating Model..." << std::endl;
        mAnimatedMesh = createSkeletonMesh(name);

        std::cout << "Total " << scene->mNumMeshes << " Meshes" << std::endl;

        std::map<unsigned int, Material*> matMap;

        int texIndex = 0;
        aiString path;  // filename

        std::cout << "Loading model materials/textures..." << std::endl;
        for (unsigned int i = 0; i < scene->mNumMaterials;i++) {
            aiMaterial* material = scene->mMaterials[i];

            Material* mat = createMaterial("basic_mat");

            if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &path) == AI_SUCCESS) {
                std::string sTextureName = std::filesystem::path(path.data).filename().string();
                Texture* tex = loadTexture("textures/" + sTextureName);
                mat->setDiffuseMap(tex);
            }

            if (material->GetTexture(aiTextureType_NORMALS, texIndex, &path) == AI_SUCCESS) {
                std::string sTextureName = std::filesystem::path(path.data).filename().string();
                Texture* tex = loadTexture("textures/" + sTextureName, true);
                mat->setNormalMap(tex);
            }

            matMap[i] = mat;
        }

        InitializeRequiredNodeMap(scene->mRootNode);


        Skeleton* skeleton = mAnimatedMesh->getSkeleton();

        skeleton->mGlobalInverseTransform = ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation);
		//skeleton->mGlobalInverseTransform = glm::inverse(skeleton->mGlobalInverseTransform);


        std::cout << "Building sub meshes..." << std::endl;

        buildMeshTransforms(scene, scene->mRootNode);

        //glm::mat4 scaleTransform(1.0);
        //scaleTransform = glm::scale(scaleTransform, glm::vec3(30, 30, 30));

        AABB boundingBox;

        for (uint32_t i = 0; i < scene->mNumMeshes;i++) {
            aiMesh* mesh = scene->mMeshes[i];
            std::string meshName(mesh->mName.data);

            std::vector<uint32_t> indices;
            std::vector<AnimatedVertex> vertices;

            vertices.reserve(mesh->mNumVertices);

            // Find the ascciated node
            aiNode* node = meshNodeIndexed[meshName];
            std::string nodeName(node->mName.data);

            glm::mat4 meshTransformation = ConvertMatrixToGLMFormat(node->mTransformation);

            //meshTransformation = glm::inverse(meshTransformation);

            //meshTransformation[0][0] = 1;
            //meshTransformation[1][1] = 1;
            //meshTransformation[2][2] = 1;

            //if (nodeName == "sleeve") {
                //std::cout << glm::to_string(meshTransformation) << std::endl;
            //}

            //printf("apply node %s transformation to mesh %s\n", node->mName.data, mesh->mName.data);

            AABB bbSubMesh;

            //std::cout << "adding vertices..." << std::endl;
            for (uint32_t v = 0; v < mesh->mNumVertices;v++) {
                aiVector3D pos = mesh->mVertices[v];
                aiVector3D uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][v] : aiVector3D(0.0f, 0.0f, 0.0f);
                aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[v] : aiVector3D(1.0f, 1.0f, 1.0f);
                aiVector3D tangent = mesh->mTangents[v];

                glm::vec4 org_pos = {pos.x, pos.y, pos.z, 1};

                glm::vec4 trans_pos = org_pos;

                //trans_pos = scaleTransform * trans_pos;

                AnimatedVertex my_v;
                my_v.position = {trans_pos.x, trans_pos.y, trans_pos.z};
                my_v.normal = {normal.x, normal.y, normal.z};
                my_v.texCoord = {uv.x, uv.y};
                my_v.tangent = {tangent.x, tangent.y, tangent.z};

                for (uint32_t k = 0;k < NUM_BONES_PER_VERTEX;k++) {
                    my_v.boneIDs[k] = 0;
                    my_v.boneWeights[k] = 0.0f;
                }
                vertices.push_back(my_v);

                bbSubMesh.extend(my_v.position);
            }

            //std::cout << "adding indices..." << std::endl;
            indices.reserve(mesh->mNumFaces * 3);
            for (uint32_t f = 0; f < mesh->mNumFaces;f++) {
                const aiFace& face = mesh->mFaces[f];

                for(int k = 0; k < 3;k++) {

                    uint32_t index = face.mIndices[k];
                    indices.push_back(index);
                }
            }

            std::map<uint32_t, uint32_t> vertexBonesCount;

            //std::cout << "Total " << mesh->mNumBones << " Bones" << std::endl;
            //std::cout << "adding bones..." << std::endl;
            for (uint32_t b = 0 ; b < mesh->mNumBones ; b++) {
                aiBone* bone = mesh->mBones[b];
                std::string boneName(bone->mName.data);

                int boneID = 0;

                //printf("bone %s, weight count %d\n", boneName.c_str(), bone->mNumWeights);

                if (mAnimatedMesh->mBoneInfoMap.find(boneName) == mAnimatedMesh->mBoneInfoMap.end()) {
                    boneID = mAnimatedMesh->mBoneCounter;
                    BoneInfo newBoneInfo;
                    newBoneInfo.id = boneID;
                    newBoneInfo.offset = ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
                    mAnimatedMesh->mBoneInfoMap[boneName] = newBoneInfo;
                    mAnimatedMesh->mBoneCounter++;
                    //printf("bone %s, ID %d\n", boneName.c_str(), boneID);
                } else {
                    boneID = mAnimatedMesh->mBoneInfoMap[boneName].id;
                    //boneIndex = boneMap[boneName];
                    //printf("duplicate bone %s\n", boneName.c_str());
                }
                assert(boneID != -1);

                for (uint32_t j = 0 ; j < bone->mNumWeights; j++) {
                    const aiVertexWeight& boneWeight = bone->mWeights[j];
                    uint32_t vertexID = boneWeight.mVertexId;
                    float weight = boneWeight.mWeight;

                    assert(vertexID <= vertices.size());
                    AnimatedVertex* vp = &vertices[vertexID];

                    bool maxWeightCrossed = true;

                    if (vertexBonesCount.find(vertexID) == vertexBonesCount.end())
                        vertexBonesCount[vertexID] = 1;
                    else
                        vertexBonesCount[vertexID]++;

                    for (uint32_t k = 0;k < NUM_BONES_PER_VERTEX;k++) {
                        if (vp->boneWeights[k] == 0.0) {
                            vp->boneWeights[k] = weight;
                            vp->boneIDs[k] = boneID;
                            maxWeightCrossed = false;
                            break;
                           // printf("vp %d, weight %f, bone %d\n", vertexID, weight, boneID);
                        }
                    }
                    if (maxWeightCrossed) {
                        printf("WARNING: v %d, total affected bones %d\n", vertexID, vertexBonesCount[vertexID]);
                    }
                    //assert(!maxWeightCrossed);
                    //uint32_t VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
                    //float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
                    //Bones[VertexID].AddBoneData(BoneIndex, Weight);
                }

                 MarkRequiredNodesForBone(bone);
            }

            Renderer* rend = Engine::get()->getRenderingSystem();

            auto vb = rend->createGPUAnimatedVertexBuffer(&vertices[0], vertices.size());
            auto ib = rend->createGPUIndexBuffer(&indices[0], indices.size());

            SubMesh* sm = mAnimatedMesh->createSubMesh(vb, ib);
            sm->tempMat = meshTransformation;
            sm->setMaterial(matMap[mesh->mMaterialIndex]);
            sm->setLocalBoundingBox(bbSubMesh);

            boundingBox.extend(bbSubMesh);
        }

        mAnimatedMesh->setBoundingBox(boundingBox);

        //std::cout << "Building skeleton..." << std::endl;
        buildSkeleton(skeleton, scene, scene->mRootNode, nullptr);
        assert(skeleton->mRootBoneList.size() > 0);
        //printf("total %d bones\n", mAnimatedMesh->mBoneInfoMap.size());
        assert(mAnimatedMesh->mBoneInfoMap.size() < MAX_BONES);

        if (scene->mNumAnimations > 0) {
            std::cout << "Creating animation..." << std::endl;
            printf("total %d animations\n", scene->mNumAnimations);
            readKeyFrames(skeleton, scene);

            skeleton->_initAnimationStates();
        }

        //skeleton->mRootBone = glm::rotate(skeleton->mRootBone, M_DEGTORAD * 90, glm::vec3( 0, 1, 0));
    }
    return mAnimatedMesh;
}

const aiNodeAnim* FindNodeAnim(const aiAnimation& Animation, const std::string& NodeName)
{
    for (uint32_t i = 0 ; i < Animation.mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = Animation.mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}

void readKeyFrames(Skeleton* skeleton, const aiScene* scene) {

    for(uint32_t animIndex = 0; animIndex < scene->mNumAnimations; animIndex++) {
        const aiAnimation& pAnimation = *scene->mAnimations[animIndex];

        printf("reading animation %s\n", pAnimation.mName.data);

        SkeletonAnimation* anim = skeleton->createAnimation(pAnimation.mName.data, pAnimation.mDuration-1, pAnimation.mTicksPerSecond);

        for (uint32_t i = 0 ; i < pAnimation.mNumChannels ; i++) {
            const aiNodeAnim* pNodeAnim = pAnimation.mChannels[i];
            std::string animNodeName = std::string(pNodeAnim->mNodeName.data);

            auto itBone = skeleton->mBoneList.find(animNodeName);

            if (itBone != skeleton->mBoneList.end()) {
                Bone* bone = itBone->second;

                BoneAnimationTrack* track = anim->createBoneAnimationTrack(bone);

                int m_NumPositions = pNodeAnim->mNumPositionKeys;
                //mPositions.reserve(m_NumPositions);
                //printf("reading animation %s for bone %s\n", pAnimation.mName.data, pNode->mName.data);

                for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
                {
                    aiVector3D aiPosition = pNodeAnim->mPositionKeys[positionIndex].mValue;
                    float timeStamp = pNodeAnim->mPositionKeys[positionIndex].mTime;

                    //printf("timeStamp %f\n", timeStamp);
                    KeyPosition data;
                    data.position = {aiPosition.x, aiPosition.y, aiPosition.z};
                    data.timeStamp = timeStamp;
                    track->insertPositionKey(data);
                    //mPositions.push_back(data);
                }

                int m_NumRotations = pNodeAnim->mNumRotationKeys;
                //mRotations.reserve(m_NumRotations);

                for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
                {
                    aiQuaternion aiOrientation = pNodeAnim->mRotationKeys[rotationIndex].mValue;
                    float timeStamp = pNodeAnim->mRotationKeys[rotationIndex].mTime;
                    KeyRotation data;
                    data.orientation = GetGLMQuat(aiOrientation);
                    data.timeStamp = timeStamp;
                    track->insertRotationKey(data);
                    //mRotations.push_back(data);
                }

                int m_NumScalings = pNodeAnim->mNumScalingKeys;
                //mScales.reserve(m_NumScalings);

                for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
                {
                    aiVector3D scale = pNodeAnim->mScalingKeys[keyIndex].mValue;
                    float timeStamp = pNodeAnim->mScalingKeys[keyIndex].mTime;
                    KeyScale data;
                    data.scale = {scale.x, scale.y, scale.z};
                    data.timeStamp = timeStamp;
                    track->insertScaleKey(data);
                    //mScales.push_back(data);
                }
            }
        }
    }
}

void buildSkeleton(Skeleton* skeleton, const aiScene* scene, const aiNode* pNode, Bone* parentBone) {
    std::string nodeName(pNode->mName.data);
    glm::mat4 nodeTransformation = ConvertMatrixToGLMFormat(pNode->mTransformation);

    Bone* boneNode = nullptr;

    std::map<std::string,NodeInfo>::iterator it = m_requiredNodeMap.find(nodeName);

    if (it == m_requiredNodeMap.end()) {
        printf("Node %s cannot be found in the required node map\n", nodeName.c_str());
        assert(0);
    }

    if (it->second.isRequired) {
        int boneId = -1;

        boneNode = skeleton->createBone(nodeName, boneId, parentBone);
        //printf("create bone %s, parent %s\n", nodeName.c_str(), parentBone ? parentBone->mName.c_str() : "root");
        // See if it's our root bone
        if (parentBone == nullptr) {
            skeleton->mRootBoneList.push_back(boneNode);
        }
        boneNode->mLocalTransform = nodeTransformation;
    } else {
        //printf("skipping node %s\n", nodeName.c_str());
    }
    for (uint32_t i = 0 ; i < pNode->mNumChildren ; i++) {
        //printf("child %d\n", i);
        buildSkeleton(skeleton, scene, pNode->mChildren[i], boneNode);
    }
}

