#include "SceneLoader.h"

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace Loader {
    void processNode(aiNode* node, const aiScene* scene) {
        for (size_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes_.push_back(this->processMesh(mesh, scene));
        }

        for (size_t i = 0; i < node->mNumChildren; i++) {
            this->processNode(node->mChildren[i], scene);
        }
    }

    bool LoadScene(Scene& scene) noexcept {
        Assimp::Importer importer;

        const aiScene* pScene = importer.ReadFile(filename,
            aiProcess_Triangulate |
            aiProcess_ConvertToLeftHanded);
        if (pScene == nullptr)
            return false;

        processNode(pScene->mRootNode, pScene);


        return true;
    }
}