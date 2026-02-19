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

	Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene) {
		// Data to fill
		std::vector<VERTEX> vertices;
		std::vector<UINT> indices;
		std::vector<Texture> textures;

		static_assert(std::is_same_v<decltype(mesh->mVertices.x), bool>);
		
		Mesh result{};

		size_t vertexBufferStrideInBytes = sizeof(DirectX::XMFLOAT4);
		if (mesh->HasNormals()) {
			result.vaMask |= VertexAttributesMask::Normals;
			vertexBufferStrideInBytes += sizeof(DirectX::XMFLOAT3);
		}
		for (size_t i = 0; i < VertexAttributesMaxTexCoords; ++i) {
			if (mesh->HasTextureCoords(i)) {
				result.vaMask |= VertexAttributesMask::TexCoords0 << i;
				vertexBufferStrideInBytes += sizeof(DirectX::XMFLOAT2);
			}
		}
		for (size_t i = 0; i < VertexAttributesMaxColors; ++i) {
			if (mesh->HasVertexColors(i)) {
				result.vaMask |= VertexAttributesMask::Color0 << i;
				vertexBufferStrideInBytes += sizeof(DirectX::XMFLOAT4);
			}
		}
		size_t vertexBufferSizeInBytes = mesh->mNumVertices * vertexBufferStrideInBytes;


		result.vertices.resize(vertexBufferSizeInBytes);

		char* vbCursor = result.vertices.data();
		for (size_t vertexIdx = 0; vertexIdx < mesh->mNumVertices; ++vertexIdx) {
			auto* position = reinterpret_cast<DirectX::XMFLOAT4*>(vbCursor);
			position->x = mesh->mVertices[vertexIdx].x;
			position->y = mesh->mVertices[vertexIdx].y;
			position->z = mesh->mVertices[vertexIdx].z;
			//TOOD: consider moving it to shader
			position->w = 1.0f;
			vbCursor += sizeof(*position);

			if (mesh->HasNormals()) {
				auto* normal = reinterpret_cast<DirectX::XMFLOAT3*>(vbCursor);
				normal->x = mesh->mNormals[vertexIdx].x;
				normal->y = mesh->mNormals[vertexIdx].y;
				normal->z = mesh->mNormals[vertexIdx].z;
				vbCursor += sizeof(*normal);
			}
			for (size_t i = 0; i < VertexAttributesMaxTexCoords; ++i) {
				if (mesh->HasTextureCoords(i)) {
					auto* texCoords = reinterpret_cast<DirectX::XMFLOAT2*>(vbCursor);
					texCoords->x = mesh->mTextureCoords[i][vertexIdx].x;
					texCoords->y = mesh->mTextureCoords[i][vertexIdx].y;
					vbCursor += sizeof(*texCoords);
				}
			}
			for (size_t i = 0; i < VertexAttributesMaxColors; ++i) {
				if (mesh->HasVertexColors(i)) {
					auto* color = reinterpret_cast<DirectX::XMFLOAT4*>(vbCursor);
					color->x = mesh->mColors[i][vertexIdx].r;
					color->y = mesh->mColors[i][vertexIdx].g;
					color->z = mesh->mColors[i][vertexIdx].b;
					color->w = mesh->mColors[i][vertexIdx].a;
					vbCursor += sizeof(*color);
				}
			}
		}

		for (UINT i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		}

		return Mesh(dev_, vertices, indices, textures);
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