#pragma once
#include "Mesh.h"
#include "renderer/Shader.h"
#include "renderer/ConstantBuffer.h"
#include "Transform.h"
#include "Texture.h"
#include "Sampler.h"
#include <string>
#include <vector>

namespace wl
{
	class Renderer;

	class Model
	{
	public:
		Model(const std::string &meshPath, const std::string &texturePath, std::shared_ptr<Shader> shader, std::string name = "unnamed model");
		~Model();
		void Draw(const Renderer &renderer) const;
		const std::shared_ptr<Shader> GetShader() const;
		std::string name{};
		Transform transform{};
	private:

		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Texture> m_texture{};
		Sampler m_sampler{};
		std::shared_ptr<Mesh> m_mesh{};
	};

}
