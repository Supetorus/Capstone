#pragma once
#include "resources/CubeMap.h"
#include "renderer/Shader.h"
#include "resources/Sampler.h"

namespace wl
{
	class Renderer;
	class CubeMap;
	class Mesh;

	class Skybox
	{
		Skybox(const std::string &texturePath);
		void Draw(const Renderer &renderer);
	private:
		std::shared_ptr<CubeMap> m_cubemap;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Mesh> m_cubeMesh;
		Sampler m_sampler;
	};

}
