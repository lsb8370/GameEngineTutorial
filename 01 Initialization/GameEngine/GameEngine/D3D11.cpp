#include "D3D11.h"

D3D11::D3D11 ()
{
}

D3D11::~D3D11 ()
{
}

bool D3D11::Initialize (HWND hWnd, unsigned int swapChainWidth, unsigned int swapChainHeight, bool isFullScreen, bool isVSync, unsigned int refreshRate, bool isMsaa, unsigned int msaaSampleCount)
{
	// ����� DirectX 11 ����
	const D3D_FEATURE_LEVEL targetFeatureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	const UINT targetFeatureLevelsCount = 2;

	// D3D11 Device ����
	UINT deviceFlags = 0;

#if _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// D3D11 Device�� �����Ѵ�.
	if (FAILED (D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, targetFeatureLevels, targetFeatureLevelsCount,
		D3D11_SDK_VERSION, m_device.ReleaseAndGetAddressOf (), &m_featureLevel, m_immediateContext.ReleaseAndGetAddressOf ())))
	{
		return false;
	}

	// �����ϴ� MSAA ǰ���� �����´�.
	UINT msaaQuality;
	m_device->CheckMultisampleQualityLevels (DXGI_FORMAT_R8G8B8A8_UNORM, msaaSampleCount, &msaaQuality);

	// DXGI ����̽��� �����´�.
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	m_device.As (&dxgiDevice);

	// DXGI ����͸� �����´�.
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter (dxgiAdapter.ReleaseAndGetAddressOf ());

	// DXGI Factory�� �����´�.
	Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
	dxgiAdapter->GetParent (__uuidof (IDXGIFactory), &dxgiFactory);

	// Swap chain ����
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { };
	swapChainDesc.BufferDesc.Width = swapChainWidth;
	swapChainDesc.BufferDesc.Height = swapChainHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = isVSync ? refreshRate : 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = isFullScreen == false;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Swap chain �����Ѵ�.
	if (FAILED (dxgiFactory->CreateSwapChain (m_device.Get (), &swapChainDesc, m_swapChain.ReleaseAndGetAddressOf ())))
	{
		return false;
	}

	// Swap chain�� Back buffer �����´�.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

	if (FAILED (m_swapChain->GetBuffer (0, __uuidof (ID3D11Texture2D), (void**)backBuffer.ReleaseAndGetAddressOf ())))
	{
		return false;
	}

	// Render target view �����.
	if (FAILED (m_device->CreateRenderTargetView (backBuffer.Get (), nullptr, m_renderTargetView.ReleaseAndGetAddressOf ())))
	{
		return false;
	}

	// Depth stencil buffer ����
	D3D11_TEXTURE2D_DESC depthStencilDescription = { };
	depthStencilDescription.Width = swapChainWidth;
	depthStencilDescription.Height = swapChainHeight;
	depthStencilDescription.MipLevels = 1;
	depthStencilDescription.ArraySize = 1;
	depthStencilDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (isMsaa)
	{
		depthStencilDescription.SampleDesc.Count = msaaSampleCount;
		depthStencilDescription.SampleDesc.Quality = msaaQuality - 1;
	}
	else
	{
		depthStencilDescription.SampleDesc.Count = 1;
		depthStencilDescription.SampleDesc.Quality = 0;
	}

	depthStencilDescription.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDescription.CPUAccessFlags = 0;
	depthStencilDescription.MiscFlags = 0;

	// Depth stencil buffer �����.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	if (FAILED (m_device->CreateTexture2D (&depthStencilDescription, nullptr, depthStencilBuffer.ReleaseAndGetAddressOf ())))
	{
		return false;
	}

	// Depth stencil view �����.
	if (FAILED (m_device->CreateDepthStencilView (depthStencilBuffer.Get (), nullptr, m_depthStencilView.ReleaseAndGetAddressOf ())))
	{
		return false;
	}

	// Output merger�� Render target �����Ѵ�.
	m_immediateContext->OMSetRenderTargets (1, m_renderTargetView.GetAddressOf (), m_depthStencilView.Get ());

	// Viewport�� �����Ѵ�.
	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<FLOAT> (swapChainWidth);
	viewport.Height = static_cast<FLOAT> (swapChainHeight);
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_immediateContext->RSSetViewports (1, &viewport);

	return true;
}

void D3D11::Shutdown ()
{
	// ���� Ÿ�� ���ε� �����Ѵ�.
	ID3D11RenderTargetView* nullRenderTargetViews[] = { nullptr };
	ID3D11DepthStencilView* nullDepthStencilView = nullptr;
	m_immediateContext->OMSetRenderTargets (1, nullRenderTargetViews, nullDepthStencilView);

	// ������ �ڿ� ���۷��� �����Ѵ�.
	m_depthStencilView = nullptr;
	m_renderTargetView = nullptr;
	m_swapChain = nullptr;

	m_immediateContext.Reset ();
	m_device.Reset ();
}

void D3D11::ClearSwapChain (float red, float green, float blue, float alpha)
{
	// Render Target�� ������ ������ �����.
	FLOAT color[4] = { red, green, blue, alpha };
	m_immediateContext->ClearRenderTargetView (m_renderTargetView.Get (), color);
}

void D3D11::PresentSwapChain (bool isVSync)
{
	// Swap chain�� Back buffer�� Front Buffer�� ��ȯ�Ѵ�.
	if (isVSync)
	{
		m_swapChain->Present (1, 0);
	}
	else
	{
		m_swapChain->Present (0, 0);
	}
}
