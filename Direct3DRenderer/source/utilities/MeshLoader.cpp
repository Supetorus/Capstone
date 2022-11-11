#include "MeshLoader.h"
#include "core/core.h"
#include "resources/Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace wl
{
	dx::XMFLOAT2 GetUVCoords(aiMesh *mesh, uint32_t index)
	{
		if (mesh->mTextureCoords[0] == nullptr)
			return { 0.0f, 0.0f };
		const auto coords = mesh->mTextureCoords[0][index];
		return { coords.x, coords.y };
	}

	std::vector<Mesh *> * MeshLoader::LoadMeshes(std::string filepath)
	{
		Assimp::Importer importer;
		auto scene = importer.ReadFile(filepath,
			aiProcess_Triangulate
			| aiProcess_JoinIdenticalVertices
			| aiProcess_GenNormals
			| aiProcess_FlipWindingOrder	//assimp uses counter clockwise winding order by default.
			| aiProcess_MakeLeftHanded		// assimp uses right handed coordinates by default.
			| aiProcess_FlipUVs				// assimp default is bottom left origin.
		);

		ASSERT(
			scene != nullptr,
			"Object failed to import.",
			importer.GetErrorString()
		);

		std::vector<Mesh*> *meshes = new std::vector<Mesh*>();
		for (unsigned int k = 0; k < scene->mNumMeshes; k++)
		{
			auto assimpMesh = scene->mMeshes[k];

			std::vector<Mesh::Vertex> vertices;
			vertices.reserve(assimpMesh->mNumVertices);
			for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
			{
				vertices.push_back(
					Mesh::Vertex
					{
						{
							assimpMesh->mVertices[i].x,
							assimpMesh->mVertices[i].y,
							assimpMesh->mVertices[i].z,
						},
						GetUVCoords(assimpMesh, i)
					}
				);
			}

			std::vector<uint16_t> indices;
			indices.reserve(assimpMesh->mNumFaces * 3);
			for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
			{
				for (unsigned int j = 0; j < 3; j++)
				{
					indices.push_back(assimpMesh->mFaces[i].mIndices[j]);
				}
			}

			Mesh *mesh = new Mesh;
			mesh->name = assimpMesh->mName.C_Str();
			mesh->SetVertices(vertices.data(), sizeof(Mesh::Vertex), static_cast<uint32_t>(vertices.size()));
			mesh->SetIndices(indices.data(), static_cast<uint32_t>(indices.size()));

			meshes->push_back(mesh);
		}
		return meshes;
	}
}
