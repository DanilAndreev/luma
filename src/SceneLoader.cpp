#include "SceneLoader.h"

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include <hlsl/ShaderTypes.hlsli>


namespace Loader {
	static Mesh processMesh(aiMesh* mesh, const aiScene* scene, DirectX::XMFLOAT4X4 transform) {
		// static_assert(std::is_same_v<decltype(mesh->mVertices.x), bool>);
		
		Mesh result{};

		if (!mesh->mName.Empty()) {
			result.name = mesh->mName.C_Str();
		}

		result.vaStride = sizeof(DirectX::XMFLOAT4);
		if (mesh->HasNormals()) {
			result.vaMask = result.vaMask | VertexAttributesMask::Normals;
			result.vaStride += sizeof(DirectX::XMFLOAT3);
		}
		if (mesh->HasTangentsAndBitangents()) {
			result.vaMask = result.vaMask | VertexAttributesMask::Tangent | VertexAttributesMask::Bitangent;
			result.vaStride += sizeof(DirectX::XMFLOAT3);
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
			if (mesh->HasTangentsAndBitangents()) {
				auto* tangent = reinterpret_cast<DirectX::XMFLOAT3*>(vbCursor);
				tangent->x = mesh->mTangents[vertexIdx].x;
				tangent->y = mesh->mTangents[vertexIdx].y;
				tangent->z = mesh->mTangents[vertexIdx].z;
				vbCursor += sizeof(*tangent);
				auto* bitangent = reinterpret_cast<DirectX::XMFLOAT3*>(vbCursor);
				bitangent->x = mesh->mBitangents[vertexIdx].x;
				bitangent->y = mesh->mBitangents[vertexIdx].y;
				bitangent->z = mesh->mBitangents[vertexIdx].z;
				vbCursor += sizeof(*bitangent);
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

		// aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		//
		// aiString str;
		// material->GetTexture(aiTextureType_NORMALS, 0, &str);
		// if (!str.Empty()) {
		// 	auto a = str;
		// }

		result.transform = transform;

		return result;
	}

	static void processNode(aiNode* node, const aiScene* assimpScene, Scene& outScene, DirectX::XMFLOAT4X4 transform) {
		for (size_t i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = assimpScene->mMeshes[node->mMeshes[i]];
			outScene.meshes.push_back(processMesh(mesh, assimpScene, transform));
		}

		for (size_t i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], assimpScene, outScene, transform);
		}
	}

    bool LoadAssetsToScene(Scene& outScene, const std::filesystem::path& filepath, DirectX::XMFLOAT4X4 transform) noexcept {
        Assimp::Importer importer;
		const auto flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
        const aiScene* pScene = importer.ReadFile(filepath.string(), flags);
		assert(pScene && (pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) == 0);
        if (pScene == nullptr)
            return false;

        processNode(pScene->mRootNode, pScene, outScene, transform);
        return true;
    }

	bool UploadSceneBuffersToGPU(Scene& scene, ID3D11Device* device) noexcept {
		for (Mesh& mesh : scene.meshes) {
			D3D11_BUFFER_DESC desc{};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.ByteWidth = mesh.vertices.size();
			desc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = mesh.vertices.data();
			device->CreateBuffer(&desc, &data, &mesh.vb);
			std::string vbName = mesh.name + ".VB";
			mesh.vb->SetPrivateData(WKPDID_D3DDebugObjectName, vbName.length(), vbName.c_str());


			desc.ByteWidth = mesh.indices.size() * sizeof(mesh.indices[0]);
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			data.pSysMem = mesh.indices.data();
			device->CreateBuffer(&desc, &data, &mesh.ib);
			std::string ibName = mesh.name + ".IB";
			mesh.ib->SetPrivateData(WKPDID_D3DDebugObjectName, ibName.length(), ibName.c_str());


			// --------- IA -------------------------------------------------------------
			UINT inputSlot = 0;
			D3D11_INPUT_ELEMENT_DESC inputElements[VertexAttributesEntriesCount] = {};
			inputElements[inputSlot].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			inputElements[inputSlot].InputSlot = inputSlot;
			inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputElements[inputSlot].InstanceDataStepRate = 0;
			inputElements[inputSlot].SemanticName = "POSITION";
			inputElements[inputSlot].SemanticIndex = 0;
			++inputSlot;

			const auto format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputElements[inputSlot].Format = format;
			inputElements[inputSlot].InputSlot = inputSlot;
			inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputElements[inputSlot].InstanceDataStepRate = 0;
			inputElements[inputSlot].SemanticName = "NORMAL";
			inputElements[inputSlot].SemanticIndex = 0;
			++inputSlot;

			inputElements[inputSlot].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputElements[inputSlot].InputSlot = inputSlot;
			inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputElements[inputSlot].InstanceDataStepRate = 0;
			inputElements[inputSlot].SemanticName = "TANGENT";
			inputElements[inputSlot].SemanticIndex = 0;
			++inputSlot;

			inputElements[inputSlot].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputElements[inputSlot].InputSlot = inputSlot;
			inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputElements[inputSlot].InstanceDataStepRate = 0;
			inputElements[inputSlot].SemanticName = "TANGENT";
			inputElements[inputSlot].SemanticIndex = 1;
			++inputSlot;

			for (size_t i = 0; i < VertexAttributesMaxTexCoords; ++i) {
				const auto format = bool(mesh.vaFlags & (VertexAttributesFlags::HalfTexCoords0 << i))
					                    ? DXGI_FORMAT_R16G16_FLOAT
					                    : DXGI_FORMAT_R32G32_FLOAT;
				inputElements[inputSlot].Format = format;
				inputElements[inputSlot].InputSlot = inputSlot;
				inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				inputElements[inputSlot].InstanceDataStepRate = 0;
				inputElements[inputSlot].SemanticName = "TEXCOORD";
				inputElements[inputSlot].SemanticIndex = i;
				++inputSlot;
			}

			for (size_t i = 0; i < VertexAttributesMaxColors; ++i) {
				const auto format = bool(mesh.vaFlags & (VertexAttributesFlags::HalfColor0 << i))
					                    ? DXGI_FORMAT_R16G16B16A16_FLOAT
					                    : DXGI_FORMAT_R32G32B32A32_FLOAT;
				inputElements[inputSlot].Format = format;
				inputElements[inputSlot].InputSlot = inputSlot;
				inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				inputElements[inputSlot].InstanceDataStepRate = 0;
				inputElements[inputSlot].SemanticName = "COLOR";
				inputElements[inputSlot].SemanticIndex = i;
				++inputSlot;
			}

			assert(inputSlot <= 15 + 1);

			const auto& shaderSrc = g_SM.GetSrc(VertexShaderID::Unity);
			device->CreateInputLayout(inputElements, inputSlot, shaderSrc.data(), shaderSrc.size(), &mesh.inputLayout);
		}

		for (size_t i = 0; i < scene.pointLights.size(); ++i) {
			auto& light = scene.pointLights[i];

			using namespace DirectX;
			light.shadowmapProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1, light.shadowMapProjNearPlane, light.shadowMapProjFarPlane);
			light.shadowmapProjInv = XMMatrixInverse(nullptr, light.shadowmapProj);
		}
		return true;
	}

	bool LoadTexture(const std::filesystem::path& path, ID3D11Device* device, ID3D11Texture2D** outTexture) noexcept {
		int width = 0, height = 0, channels = 0;
		auto status = stbi_info(path.string().c_str(), &width, &height, &channels);
		assert(status);
		channels = channels == 3 ? 4 : channels;

		stbi_us *img = stbi_load_16(path.string().c_str(), &width, &height, nullptr, channels);
		if (img == NULL) {
			printf("Error in loading the image\n");
			exit(1);
		}

		assert(channels <= 4);
		assert(channels > 0);

		D3D11_TEXTURE2D_DESC desc{};
		switch (channels) {
			case 1:
				desc.Format = DXGI_FORMAT_R16_SNORM;
				break;
			case 2:
				desc.Format = DXGI_FORMAT_R16G16_SNORM;
				break;
			case 3:
			default:
				desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
		}
		desc.ArraySize = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		D3D11_SUBRESOURCE_DATA payload{};
		payload.pSysMem = img;
		payload.SysMemPitch = width * sizeof(img[0]) * channels;

		device->CreateTexture2D(&desc, &payload, outTexture);
		stbi_image_free(img);
		return true;
	}
}
