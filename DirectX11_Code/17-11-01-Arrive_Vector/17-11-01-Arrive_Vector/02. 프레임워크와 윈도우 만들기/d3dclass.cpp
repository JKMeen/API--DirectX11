////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "d3dclass.h"


D3DClass::D3DClass()
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}

// ��ü �ʱ�ȭ
// DirectX 11�� ��ü Direct3D ������ �Ͼ�� ��
// SystemClass���� ���� �������� �ʺ�� ���̸�
// Direct3D�� �̿��Ͽ� ������ ũ���� ������ �ʱ�ȭ�ϰ� �̿�
// hWnd ������ ������� �����쿡 ���� �ڵ�
// sereenDepth, screenNear : �����쿡 �׷��� 3D ȯ�濡���� ���� ��
// vsync : Direct3D �������� ������� ���ΰ�ħ ������ ���� ������, ������ �ٽ� �׸� ������ ����
bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, 
						  float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;


	// Store the vsync setting.
	// vsync(��������ȭ) ������ ����
	m_vsync_enabled = vsync;

	// Create a DirectX graphics interface factory.
	// DirectX �׷��� �������̽� ���丮�� ����
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if(FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	// ���丮 ��ü�� ����Ͽ� ù ��° �׷��� ī�� �������̽��� ���� ����͸� ����
	result = factory->EnumAdapters(0, &adapter);
	if(FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	// ��� (�����)�� ���� ù ��° ����͸� ����
	result = adapter->EnumOutputs(0, &adapterOutput);
	if(FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	// DXGI_FORMAT_R8G8B8A8_UNORM ����� ��� ���÷��� ���信 �´� ����� ������ ����
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	// ������ ��� ����Ϳ� �׷��� ī�� ������ ������ ����Ʈ�� ����
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	// ���÷��� ��忡 ���� ����Ʈ ������ ä������
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	// ��� ���÷��� ��忡 ���� ȭ�� �ʺ�/���̿� �´� ���÷��̸� ã��,
	// ������ ���� ã���� ������� ���ΰ�ħ ������ �и�� ���� ���� ����
	for(i=0; i<numModes; i++)
	{
		if(displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if(displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	// �����(�׷���ī��)�� description�� �����ɴϴ�.
	result = adapter->GetDesc(&adapterDesc);
	if(FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	// ���� �׷���ī���� �޸� �뷮�� �ް�����Ʈ ������ ����
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	// �׷���ī���� �̸��� char�� ���ڿ� �迭�� �ٲ� �� ����
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if(error != 0)
	{
		return false;
	}


	/////////////////////////////////////////////////////
	// ����� ���ΰ�ħ ������ ����/�и� ���� �׷���ī���� ������ �ֱ� ������
	// ������ ��� ���� ����� ����ü��� �������̽����� �ݳ�
	////////////////////////////////////////////////////////////////////////////

	// Release the display mode list.
	// ���÷��� ��� ����Ʈ�� �Ҵ��� ����
	delete [] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	// ��� ����͸� �Ҵ� ����
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	// ����͸� �Ҵ� ����
	adapter->Release();
	adapter = 0;

	// Release the factory.
	// ���丮 ��ü�� �Ҵ� ����
	factory->Release();
	factory = 0;


	///////////////////////////////////////////////////////////////////////////
	// ���ΰ�ħ ������ ������Ƿ� DirectX�� �ʱ�ȭ�� ����
	// ���� ���� ���� ü���� description ����ü�� ä�� ����
	// ���� ü���� ����Ʈ���ۿ� ����۷� �̷���� �ִ� ������ �������� �ϸ� ���� ��
	// �ϳ��� ����۸��� ����Ͽ� �� ���� �׸� �� ����Ʈ ���ۿ� �ٲ�ġ�� �� : ���� ü��
	////////////////////////////////////////////////////////////////////////////

	// Initialize the swap chain description.
	// ���� ü�� description�� �ʱ�ȭ ��
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	// �ϳ��� ����۸��� ���
    swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	// ������� �ʺ�� ���̸� ����
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
	// ����۷� �Ϲ����� 32bit�� �����̽��� ����
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	/////////////////////////////////////////////////////////////////////////
	// ���� ü�� ����ü ä��� �� ���ΰ�ħ ������ ����
	// ���ΰ�ħ ������ �ʴ� �� ���� ����۸� �׷� ����Ʈ ���ۿ� �ٲ�ġ���ϴ����� ��Ÿ���� ����
	// graphicsclass.h�� vsync ������ true�� �����Ǿ� ������ �ý��ۿ��� ����
	// ���ΰ�ħ ������ ����(ex: 60hz), �ʴ� 60�� ȭ���� �׸�
	// vsync ������ false�� ��� �ִ��� ���� ȭ���� �׷������� �� > �ܻ��� ����
	////////////////////////////////////////////////////////////////////////////

	// Set the refresh rate of the back buffer.
	// ������� ���ΰ�ħ ������ ����
	if(m_vsync_enabled)
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	// ������� �뵵�� ����
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	// �������� �̷���� �������� �ڵ��� ����
    swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	// ��Ƽ���ø��� ��
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	// ������ ��� �Ǵ� Ǯ��ũ�� ��带 ����
	if(fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	// ��ĵ������ ���İ� ��ĵ���̴��� �������� ����(unspecified)���� ����
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	// ��µ� ������ ������� ������ �������� ��
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	// �߰� �ɼ� �÷��׸� ������� ����
	swapChainDesc.Flags = 0;


	//////////////////////////////////////////////////////////////////////////
	// ���� ü�� description ����ü�� ä��� ���Ҿ� ���� �����̶�� ������ ����
	// �� ������ �츮�� ��� ������ DirectX�� ����� ������ �˷���
	// ������ 11.0���� �����ϰ�, �ٸ� ������ ���� ����
	////////////////////////////////////////////////////////////////////////////

	// Set the feature level to DirectX 11.
	// ���� ������ DirectX 11�� ����
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	////////////////////////////////////////////////////////////////////////////
	// Direct3D ����̽��� Direct3D����̽� ���ٽ�Ʈ�� ��� Direct3D ����� �������̽��� �Ǳ� ������ �߿���
	// ����̽��� ����̽� ���ؽ�Ʈ�� ����Ͽ� ��κ��� �۾��� ����
	// ������ DirectX11�� �����ϴ� �׷���ī�带 ������ ���� �ʴٸ�
	// ����̽��� ����̽� ���ؽ�Ʈ�� �����ϴ� �Լ��� ������ ����,
	// ���� DirectX 11�� ����� �׽�Ʈ�غ���� ������ �׷���ī�尡 DirectX 11��
	// �������� �ʴ� ��� �� �κп��� D3D_DRIVER_TYPE_HARDWARE�� D3D_DRIVER_TYPE_REFERENCE�� �ٲپ
	// �������� CPU���� ó���ϰ� �� �� ����
	// �ӵ��� 1/1000 ������ �������� DirectX11�� �������� �ʴ� �׷��� ī�带 ���� ����鿡�� ���� �����
	////////////////////////////////////////////////////////////////////////////

	// Create the swap chain, Direct3D device, and Direct3D device context.
	// ���� ü��, Direct3D ����̽�, Direct3D ����̽� ���ؽ�Ʈ�� ����
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, 
										   D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if(FAILED(result))
	{
		return false;
	}


	////////////////////////////////////////////////////////////////////////////
	// �⺻ �׷��� ī��� �������� �ʰ� ��� ���� ī����� �����Ͽ� ���� �� �´� �׷��� ī��� ���� �� �� �ֵ��� ��
	// ���� ü���� �������Ƿ� ����۸� ����� �� �����͸� ���� ü�ο� ��������־�� ��
	// CreateRenderTargetView �Լ��� ����Ͽ� ����۸� ���� ü�ο� ������
	////////////////////////////////////////////////////////////////////////////

	// Get the pointer to the back buffer.
	// ������� �����͸� ������
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if(FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	// ������� �����ͷ� ����Ÿ�� �並 ����
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if(FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	// ����� �����͸� �� �̻� ������� �����Ƿ� �Ҵ� ����
	backBufferPtr->Release();
	backBufferPtr = 0;


	////////////////////////////////////////////////////////////////////////////
	// ���� ������ description ����ü�� �ۼ��ؾ� ��
	// ���� ���۸� ������ 3D �������� ��������� �ùٸ��� �׷���
	// ���� ���ÿ� ���ٽ� ���۵� �� ���� ���ۿ� ������
	// ���ٽ� ���۴� ��� ���� �����ִ� �׸��� ���� ȿ���� ��Ÿ�� �� ���
	////////////////////////////////////////////////////////////////////////////

	// Initialize the description of the depth buffer.
	// ���� ������ description�� �ʱ�ȭ ��
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	// ���� ������ dscription�� �ۼ���
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;


	////////////////////////////////////////////////////////////////////////////
	// �� ������ �̿��Ͽ� ����/���ٽ� ������ description�� ���� �� �ְ� ��
	// CreateTexture2D �Լ��� �̿������� ���۴� 2D �ؽ��Ķ�� ���� �� �� ����
	// ��������� ���ĵǰ� ������ȭ�� ���Ŀ��� 2D��ǥ�� �ȼ����� �Ǳ� ������
	////////////////////////////////////////////////////////////////////////////

	// Create the texture for the depth buffer using the filled out description.
	// description�� ����Ͽ� ���� ������ �ؽ��ĸ� ������
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if(FAILED(result))
	{
		return false;
	}


	////////////////////////////////////////////////////////////////////////////
	// ����-���ٽ� description�� �ۼ��� ���̸�,
	// Direct3D���� �� �ȼ��� � ���� �׽�Ʈ�� �� ������ ���� �� �ְ� ����
	////////////////////////////////////////////////////////////////////////////

	// Initialize the description of the stencil state.
	// ���ٽ� ������ description�� �ʱ�ȭ ��
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	// ���ٽ� ������ description�� �ۼ���
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	// �ȼ��� �տ� ���� ��� ��ȯ
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	// �ȼ��� �ڿ� ���� ��� ��ȯ
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	////////////////////////////////////////////////////////////////////////////
	// description�� ���� ����-���ٽ� ���� ������ ����
	////////////////////////////////////////////////////////////////////////////

	// Create the depth stencil state.
	// ����-���ٽ� ���¸� ����
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if(FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	// ����-���ٽ� ���¸� ����
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);


	////////////////////////////////////////////////////////////////////////////
	// ���� ����-���ٽ� ������ �信 ���� description�� ����
	// �� �۾��� �ؾ� Direct3D���� ���� ���۸� ����-���ٽ� �ؽ��ķ� �ν���
	// �� ����ü�� ä��� �� �Ŀ� CreateDepthStencilView �Լ��� ȣ���Ͽ� ������
	////////////////////////////////////////////////////////////////////////////

	// Initialize the depth stencil view.
	// ����-���ٽ� ���� description�� �ʱ�ȭ ��
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	// ����-���ٽ� ���� description�� �ۼ���
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	// ����-���ٽ� �並 ������
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if(FAILED(result))
	{
		return false;
	}


	////////////////////////////////////////////////////////////////////////////
	// OMSetRenderTarget �Լ��� ȣ���� �� �ְ� ��
	// OMSetRenderTarget : ����Ÿ�ٺ�� ����-���ٽ� �並 ��� ������ ���������ο� ���ε� ������
	// ������������ �̿��� �������� ����� �� �츮�� ������� ����ۿ� �׷����� ��
	// ����ۿ� �׷��� ���� ����Ʈ ���ۿ� �ٲ�ġ���Ͽ� ������ ����Ϳ� �����
	////////////////////////////////////////////////////////////////////////////

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	// ����Ÿ�� ��� ����-���ٽ� ���۸� ���� ��� ���������ο� ���ε���
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);


	////////////////////////////////////////////////////////////////////////////
	// ���� Ÿ���� ������ �� �߰����� ��ɵ��� ������ �� ����
	// ù��°�� ����� �Ǵ� ���� ������ȭ�� ����(rasterizer state)
	// �̰��� ������ ��� �׷��������� ���� ��� ���� �� �ְԵ�
	// �̰��� �̿��Ͽ� ȭ���� ���̾������� ���� �׸��ų� ������ �յ޸��� ��� �׸����� �� �� ����
	// �⺻������ DirectX���� �����ǰ� ���ư��� ������ȭ�� ���°� ������ ���� ������ ������ ������� ����
	////////////////////////////////////////////////////////////////////////////

	// Setup the raster description which will determine how and what polygons will be drawn.
	// � ������ ��� �׸� ������ �����ϴ� ������ȭ�� description�� �ۼ�
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	// �ۼ��� description���κ��� ������ȭ�� ���¸� ����
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if(FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	// ������ȭ�� ���¸� ����
	m_deviceContext->RSSetState(m_rasterState);
	

	////////////////////////////////////////////////////////////////////////////
	// ����Ʈ�� �־�� ����Ÿ�� �������� Ŭ������ ������ �� ����
	// �켱 �̰��� ������ ũ��� �����ϰ� ������
	////////////////////////////////////////////////////////////////////////////

	// Setup the viewport for rendering.
	// �������� ���� ����Ʈ�� ����
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

	// Create the viewport.
	// ����Ʈ�� ����
    m_deviceContext->RSSetViewports(1, &viewport);


	////////////////////////////////////////////////////////////////////////////
	// ���� �������(projection matrix)�� ������ ����
	// ���� ����� 3D�� ȭ���� �ռ� ������� 2D ����Ʈ �������� ��ȯ�ϵ��� ����
	// �� ����� ���纻�� ����� ���̴����� ����� �� �ֵ��� �ؾ� ��
	////////////////////////////////////////////////////////////////////////////

	// Setup the projection matrix.
	// ���� ����� ������
	fieldOfView = (float)D3DX_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	// 3D �������� ���� ���� ����� ������
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);


	////////////////////////////////////////////////////////////////////////////
	// ���� ����̶�� �Ǵٸ� ����� ������ ��
	// �� ����� �츮�� ������Ʈ���� 3D ������ ��ǥ�� ��ȯ�ϴµ� ����
	// ���� 3�����󿡼��� ȸ��/�̵�/ũ�� ��ȯ������ ��� ��
	// ���纻�� ����� ���̴����� ����� �� �ְ� ��
	////////////////////////////////////////////////////////////////////////////

    // Initialize the world matrix to the identity matrix.
	// ���� ����� ���� ��ķ� �ʱ�ȭ��
    D3DXMatrixIdentity(&m_worldMatrix);


	////////////////////////////////////////////////////////////////////////////
	// ���� ����� �� ����� �����ϴ� ���� ��, �� ����� ���� ��鿡�� �츮��
	// ���� ���� �ִ°��� ����ϴ� �� �̿��
	// �׷� ���� �ϴ� ���̶�� ���� ī�޶� �����س� �� ����
	// Initalize �Լ����� ���������� �ؾ��� ���� ���翵 ��� �����
	// ���翵 ����� 3D��ü�� �ƴ϶� UI�� ���� 2D�� ��ҵ��� �׸��� ���� ����
	////////////////////////////////////////////////////////////////////////////

	// Create an orthographic projection matrix for 2D rendering.
	// 2D �������� ���� ���翵 ����� ������
	D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);

    return true;
}

// ��ü ���� (���� �� ���)
// Initalize �Լ����� ����ߴ� �����͵��� �����ϰ� �ݳ��ϴ� ���� ��
// �� �۾��� �ϱ� ���� ���� ü���� ������ ���� �ٲٴ� �Լ��� ȣ����
// �� �۾��� �̷������ �ʴ´ٸ� Ǯ��ũ�� ���¿��� ���� ü���� ������ �Ͼ ��
// ��� ���ܰ� �߻��ϰ� ��
// ���� �׷� ���ܸ� ���ϱ� ���� Direct3D�� �����ϱ� ���� ������ ���� ü���� ������ ���� �ٲ��־�� ��
void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	// �����ϱ� ���� �̷��� ������ ��带 �ٲ��� ������ ����ü���� �Ҵ� ������ �� ���ܰ� �߻���
	if(m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if(m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if(m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if(m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if(m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if(m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if(m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if(m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if(m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}


// ���ο� �� �����Ӹ��� 3D ȭ���� �׸��� ������ �� ȣ���
// ���۸� �� ������ �ʱ�ȭ�ϰ� �������� �̷�������� �غ���
void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	// ���۸� � �������� ���� ������ ����
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	// ������� ������ ����
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
    
	// Clear the depth buffer.
	// ���� ���۸� ����
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


// �� �������� �������� ���� ü�ο��� ����ۿ� �׸� 3D ȭ���� ǥ���ϵ��� �ϰ� ����
void D3DClass::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	// �������� �Ϸ�Ǿ����Ƿ� ������� ������ ȭ�鿡 ǥ����
	if(m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		// ���ΰ�ħ ������ ����
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		// ������ �� ������ ǥ����
		m_swapChain->Present(0, 0);
	}

	return;
}


// Direct3D ����̽��� �����͸� ������
ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}

// Direct3D ����̽� ���ؽ�Ʈ�� �����͸� ������
ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

// ���� ����� ��ȯ
void D3DClass::GetProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}

// ���� ����� ��ȯ
void D3DClass::GetWorldMatrix(D3DXMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}

// ���翵 ����� ��ȯ
void D3DClass::GetOrthoMatrix(D3DXMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

// �׷���ī���� �̸��� ��밡���� �޸��� ���� ��ȯ
// �׷��� ī���� �̸��� �޸��� ũ�⸦ �ƴ� ���� ���� �ٸ� �������� ������ϴµ� ������ ��
void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}