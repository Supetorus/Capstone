#pragma once
#include "core/core.h"
#include "Shader.h"
#include "d3d11.h"
namespace wl
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer(uint32_t size, ShaderStage shaderStage);
		void SetData(const void *data);
		void Bind(uint32_t slot);
	private:
		wrl::ComPtr<ID3D11Buffer> m_cBuffer;
		ShaderStage stage;
		uint32_t size;
	};
}

