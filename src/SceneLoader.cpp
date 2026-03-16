#include "SceneLoader.h"

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <hlsl/ShaderTypes.hlsli>


namespace Loader {
	static Mesh processMesh(aiMesh* mesh, const aiScene* scene, DirectX::XMFLOAT4X4 transform) {
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
		const auto flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
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

			desc.ByteWidth = mesh.indices.size() * sizeof(mesh.indices[0]);
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			data.pSysMem = mesh.indices.data();
			device->CreateBuffer(&desc, &data, &mesh.ib);


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

			if (bool(mesh.vaMask & VertexAttributesMask::Normals)) {
				const auto format = bool(mesh.vaFlags & VertexAttributesFlags::HalfNormals)
					                    ? DXGI_FORMAT_R16G16B16A16_FLOAT
					                    : DXGI_FORMAT_R32G32B32A32_FLOAT;
				inputElements[inputSlot].Format = format;
				inputElements[inputSlot].InputSlot = inputSlot;
				inputElements[inputSlot].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				inputElements[inputSlot].InstanceDataStepRate = 0;
				inputElements[inputSlot].SemanticName = "NORMAL";
				inputElements[inputSlot].SemanticIndex = 0;
				++inputSlot;
			}

			for (size_t i = 0; i < VertexAttributesMaxTexCoords; ++i) {
				if (bool(mesh.vaMask & (VertexAttributesMask::TexCoords0 << i))) {
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
			}

			for (size_t i = 0; i < VertexAttributesMaxColors; ++i) {
				if (bool(mesh.vaMask & (VertexAttributesMask::Color0 << i))) {
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
			}
			assert(inputSlot <= 15 + 1);

			const auto& shaderSrc = g_SM.GetSrc(VertexShaderID::Unity);
			device->CreateInputLayout(inputElements, inputSlot, shaderSrc.data(), shaderSrc.size(), &mesh.inputLayout);
		}

		for (size_t i = 0; i < scene.pointLights.size(); ++i) {
			auto& light = scene.pointLights[i];

			D3D11_TEXTURE2D_DESC desc{};
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
			desc.Width = LUMA_OMNIDIR_SHADOW_MAP_DIM;
			desc.Height = LUMA_OMNIDIR_SHADOW_MAP_DIM;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ArraySize = 6;
			desc.MipLevels = 1;
			desc.SampleDesc.Count = 1;
			device->CreateTexture2D(&desc, nullptr, &light.m_ShadowCubemap);

			//TODO: use GS to render it

			// D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			// dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			// dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			// dsvDesc.Flags = 0;
			// dsvDesc.Texture2DArray.MipSlice = 0;
			// dsvDesc.Texture2DArray.FirstArraySlice = 0;
			// dsvDesc.Texture2DArray.ArraySize = 1;
			// device->CreateDepthStencilView(light.m_ShadowCubemap, &dsvDesc, &light.m_ShadowCubemapDSV);

			for (size_t cubemapFaceIdx = 0; cubemapFaceIdx < std::size(light.m_ShadowCubemapDSV); ++cubemapFaceIdx) {
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Flags = 0;
				dsvDesc.Texture2DArray.MipSlice = 0;
				dsvDesc.Texture2DArray.FirstArraySlice = cubemapFaceIdx;
				dsvDesc.Texture2DArray.ArraySize = 1;
				device->CreateDepthStencilView(light.m_ShadowCubemap, &dsvDesc, &light.m_ShadowCubemapDSV[cubemapFaceIdx]);
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D10_1_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1;

			device->CreateShaderResourceView(light.m_ShadowCubemap, &srvDesc, &light.m_ShadowCubemapSRV);

			using namespace DirectX;
			using namespace DirectX;
			light.shadowmapProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1, light.shadowMapProjNearPlane, light.shadowMapProjFarPlane);
			light.shadowmapProjInv = XMMatrixInverse(nullptr, light.shadowmapProj);
		}
		return true;
	}
}
