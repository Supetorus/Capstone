#pragma once
#include <memory>
#include <vector>
#include <string>

namespace wl
{
	class Mesh;

	class MeshLoader
	{
	public:
		static std::shared_ptr<Mesh> LoadMesh(std::string filepath);
	};

}
