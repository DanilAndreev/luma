#include "SceneLoader.h"

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace Loader {
	static Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		// static_assert(std::is_same_v<decltype(mesh->mVertices.x), bool>);
		
		Mesh result{};

		result.vaStride = sizeof(DirectX::XMFLOAT4);
		if (mesh->HasNormals()) {
			result.vaMask = result.vaMask | VertexAttributesMask::Normals;
			result.vaStride += sizeof(DirectX::XMFLOAT3);
		}
		for (size_t i = 0; i < VertexAttributesMaxTexCoords; ++i) {
			if (mesh->HasTextureCoords(i)) {
				result.vaMask = result.vaMask | VertexAttributesMask::TexCoords0 << i;
				result.vaStride += sizeof(DirectX::XMFLOAT2);
			}
		}
		for (size_t i = 0; i < VertexAttributesMaxColors; ++i) {
			if (mesh->HasVertexColors(i)) {
				result.vaMask = result.vaMask | VertexAttributesMask::Color0 << i;
				result.vaStride += sizeof(DirectX::XMFLOAT4);
			}
		}
		size_t vertexBufferSizeInBytes = mesh->mNumVertices * result.vaStride;


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

		result.indices.reserve(mesh->mNumFaces * 3);
		for (size_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			for (size_t j = 0; j < face.mNumIndices; ++j) {
				result.indices.push_back(face.mIndices[j]);
			}
		}

		//if (mesh->mMaterialIndex >= 0) {
		//	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		//	std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
		//	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		//}

		DirectX::XMStoreFloat4x4(&result.transform, DirectX::XMMatrixIdentity());
		return result;
	}

	static void processNode(aiNode* node, const aiScene* assimpScene, Scene& outScene) {
		for (size_t i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = assimpScene->mMeshes[node->mMeshes[i]];
			outScene.meshes.push_back(processMesh(mesh, assimpScene));
		}

		for (size_t i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], assimpScene, outScene);
		}
	}

    bool LoadScene(Scene& outScene) noexcept {
        Assimp::Importer importer;

		const auto flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals;
        const aiScene* pScene = importer.ReadFile("assets/stanford-bunny.obj", flags);
        if (pScene == nullptr)
            return false;

        processNode(pScene->mRootNode, pScene, outScene);


        return true;
    }
}