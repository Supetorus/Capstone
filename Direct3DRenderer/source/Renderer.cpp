#include "Renderer.h"
#include "windows/Window.h"
#include "core/Log.h"

#include <d3dcompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

namespace wl
{
	Renderer::Renderer(const Window &window) : m_window(window)
	{
		createDevice();
		createSwapChain();
		createRenderTarget();
		createDepthStencilBuffer();
		bindRenderTargets();
		createViewport();

		LOG("Renderer created");
	}

	void Renderer::RenderFrame()
	{

		// Clear color.
		float color[] = {1.0f, 1.0f, 1.0f, 1.0f};
		m_context->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
		// Clear depth
		m_context->ClearDepthStencilView(m_pDepthStencilView.Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f, 0);

//Test Triangle stuff
		//Vertex buffer
		struct Vertex
		{
			float x;
			float y;
			//float r;
			//float g;
			//float b;
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};

		const Vertex vertices[]
		{
			{  0.0f, 0.5f,255,  0,  0,  0 },
			{  0.5f,-0.5f,  0,255,  0,  0 },
			{ -0.5f,-0.5f,  0,  0,255,  0 },
			{ -0.3f, 0.3f,  0,255,  0,  0 },
			{  0.3f, 0.3f,  0,  0,255,  0 },
			{  0.0f,-0.8f,255,  0,  0,  0 },
		};

		wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
		D3D11_BUFFER_DESC vbd{};
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.CPUAccessFlags = 0u;
		vbd.MiscFlags = 0u;
		vbd.ByteWidth = sizeof(vertices);
		vbd.StructureByteStride = sizeof(Vertex);
		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = vertices;
		ASSERT_HR(m_device->CreateBuffer(&vbd, &sd, pVertexBuffer.GetAddressOf()), "Failed to create vertex buffer.");

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0u;
		m_context->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);

		uint16_t indices[]{
			0, 1, 2,
			0, 2, 3,
			0, 4, 1,
			2, 1, 5,
		};
		wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
		D3D11_BUFFER_DESC ibd{};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.ByteWidth = sizeof(indices);
		ibd.StructureByteStride = sizeof(uint16_t);
		D3D11_SUBRESOURCE_DATA isd{};
		isd.pSysMem = indices;
		ASSERT_HR(m_device->CreateBuffer(&ibd, &isd, pIndexBuffer.GetAddressOf()),
			"Failed to create index buffer.");

		m_context->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

		wrl::ComPtr<ID3DBlob> pBlob;
		//Pixel Shader
		wrl::ComPtr<ID3D11PixelShader> pPixelShader;
		ASSERT_HR(D3DReadFileToBlob( L"../Assets\\Shaders\\PixelShader.cso", &pBlob), "Unable to read in pixel shader.");
		ASSERT_HR(m_device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf()), "Unable to create Pixel Shader.");
		m_context->PSSetShader(pPixelShader.Get(), nullptr, 0u);

		//Vertex Shader
		wrl::ComPtr<ID3D11VertexShader> pVertexShader;
		ASSERT_HR(D3DReadFileToBlob(L"../Assets/Shaders/VertexShader.cso", &pBlob), "Unable to read Vertex Shader.");
		ASSERT_HR(m_device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf()), "Unable to create vertex shader.");
		//Bind vertex shader
		m_context->VSSetShader(pVertexShader.Get(), nullptr, 0u);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//Vertex input layout
		wrl::ComPtr<ID3D11InputLayout> pInputLayout;
		D3D11_INPUT_ELEMENT_DESC elemDescs[]
		{
			{"Position", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Color",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		ASSERT_HR(m_device->CreateInputLayout(elemDescs, static_cast<UINT>(std::size(elemDescs)), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout),
			"Failed to create input layout.");

		m_context->IASetInputLayout(pInputLayout.Get());

		//Draw
		m_context->DrawIndexed(static_cast<UINT>(std::size(indices)), 0u, 0);
		
//End Test Triangle
		bindRenderTargets();

		//Present
		ASSERT_HR(m_pDXGISwapChain->Present(0, 0),
			"Error presenting frame.");
	}

	void Renderer::createDevice()
	{
		D3D_FEATURE_LEVEL levels[] = {
				D3D_FEATURE_LEVEL_9_1,
				D3D_FEATURE_LEVEL_9_2,
				D3D_FEATURE_LEVEL_9_3,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1
		};
		// This flag adds support for surfaces with a color-channel ordering different
		// from the API default. It is required for compatibility with Direct2D.
		UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		// Create the Direct3D 11 API device object and a corresponding context.
		wrl::ComPtr<ID3D11Device> device;
		wrl::ComPtr<ID3D11DeviceContext> context;
		HRESULT hr = D3D11CreateDevice(
			nullptr, // Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE, // Create a device using the hardware graphics driver.
			0, // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			deviceFlags, // Set debug and Direct2D compatibility flags.
			nullptr, // nullptr means it will target the latest available. should be 11.
			0, // Size of the list above.
			D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&device, // Returns the Direct3D device created.
			&m_featureLevel, // Returns feature level of device created.
			&context // Returns the device immediate context.
		);
		if (FAILED(hr))
		{
			LOG("Failed to create device.");
		}
		// Store pointers to the Direct3D 11.1 API device and immediate context.
		device.As(&m_device);
		context.As(&m_context);
		LOG("Device Created.");
	}

	void Renderer::createSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC desc;								// A struct which describes the swap chain.
		ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));		// Clears the memory to zero out all the values.
		desc.BufferDesc.Width = m_window.GetClientSize().first;
		desc.BufferDesc.Height = m_window.GetClientSize().second;
		desc.BufferDesc.RefreshRate.Numerator = 60;				// These were not explained in the book.
		desc.BufferDesc.RefreshRate.Denominator = 1;			// These were not explained in the book.
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;// These were not explained in the book.
		desc.Windowed = TRUE;									// Sets the initial state of full-screen mode.
		desc.BufferCount = 1;									// 1 Back buffer.
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// The pixel format of the back buffer.
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// I will be rendering to the back buffer.
		desc.SampleDesc.Count = 1;								// multisampling setting
		desc.SampleDesc.Quality = 0;							// vendor-specific flag
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;				// Specify DXGI_SWAP_EFFECT_DISCARD in order to
		//	let the display driver select the most efficient
		//	presentation method.
		desc.OutputWindow = m_window.GetHandle();

		// Get the Factory used to create the ID3D11Device
		wrl::ComPtr<IDXGIDevice> dxgiDevice;
		ASSERT_HR(m_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)), "Failed to get IDXGIDevice.");

		wrl::ComPtr<IDXGIAdapter> adapter;
		ASSERT_HR(dxgiDevice->GetParent(IID_PPV_ARGS(&adapter)), "Failed to get IDXGIAdapter.");

		wrl::ComPtr<IDXGIFactory> factory;
		ASSERT_HR(adapter->GetParent(IID_PPV_ARGS(&factory)), "Failed to get IDXGIFactory.");

		ASSERT_HR(
			factory->CreateSwapChain(
				m_device.Get(),
				&desc,
				&m_pDXGISwapChain
			), "Failed to create swap chain.");
		LOG("Swap chain created");
	}

	void Renderer::createRenderTarget()
	{
		// Get the back buffer from the swap chain
		ASSERT_HR(
			m_pDXGISwapChain->GetBuffer(
				0,
				IID_PPV_ARGS(&m_pBackBuffer)),
			"Unable to get back buffer.");

		// Create the render target view bound to the back buffer resource.
		ASSERT_HR(
			m_device->CreateRenderTargetView(
				m_pBackBuffer.Get(),
				nullptr,
				m_pRenderTargetView.GetAddressOf()),
			"Unable to create RenderTargetView.");

		m_pBackBuffer->GetDesc(&m_bbDesc);
		LOG("Render target view created");
	}

	void Renderer::createDepthStencilBuffer()
	{
		//A depth-stencil buffer is just a particular form of ID3D11Texture2D
		//	resource, which is typically used to determine which pixels have draw priority during rasterization based on the
		//	distance of the objects in the scene from the camera.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			m_window.GetClientSize().first,
			m_window.GetClientSize().second,
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);
		ASSERT_HR(
			m_device->CreateTexture2D(
				&depthStencilDesc,
				nullptr,
				&m_pDepthStencil),
			"Failed to create depth/stencil buffer.");
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		ASSERT_HR(
			m_device->CreateDepthStencilView(
				m_pDepthStencil.Get(),
				0, //"Because we specified the type of our depth/stencil buffer, we specify null for this parameter."
				&m_pDepthStencilView),
			"Failed to create depth/stencil view");
		LOG("Depth/stencil buffer created");
	}

	void Renderer::bindRenderTargets()
	{
		m_context->OMSetRenderTargets(
			1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
	}

	void Renderer::createViewport()
	{
		ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
		m_viewport.Width = static_cast<float>(m_window.GetClientSize().first);
		m_viewport.Height = static_cast<float>(m_window.GetClientSize().second);
		m_viewport.MinDepth = 0;
		m_viewport.MaxDepth = 1;
		m_context->RSSetViewports(
			1, // More than 1 can be used for advanced effects.
			&m_viewport
		);
	}
}