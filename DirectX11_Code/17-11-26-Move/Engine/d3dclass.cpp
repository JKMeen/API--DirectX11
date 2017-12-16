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

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLength = 0;
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


	// vsync 설정 저장.
	m_vsync_enabled = vsync;

		// DirectX 그래픽스 인터페이스 팩토리 생성.
		result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// 비디오 카드로 적용할 adapter 생성을 위해 팩토리 사용.
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// 기본 adapter 출력(모니터) 열거.
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// adapter에 대한DXGI_FORMAT_R8G8B8A8_UNORM 출력 형식을 맞추는 출력 모드 개수를 얻는다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 모니터/비디오 카드 둘 다 가능한 모든 출력 모드를 가지는 리스트 생성.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// 리스트에 앞서 구한 출력 모드를 채운다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// 모든 출력모드를 확인하여 화면 너비, 화면 높이와 일치하는 하나를 찾는다.
	// 만약 찾으면 해당 모니터의 재생률의 분자, 분모를 저장한다.
	for (i = 0; i<numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

		// adapter (비디오 카드) 설명은 얻는다.
		result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// 비디오 카드 전용 메모리를 메가바이트(MB) 단위로 저장한다.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// 비디오 카드 이름을 문자열로 바꾸고 저장.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

		// 화면 모드 리스트 해제.
		delete[] displayModeList;
	displayModeList = 0;

	// adapter output 해제.
	adapterOutput->Release();
	adapterOutput = 0;

	// adapter 해제.
	adapter->Release();
	adapter = 0;

	// 팩토리 해제.
	factory->Release();
	factory = 0;

		// 스왑 체인 description을 초기화한다.
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// 단일 후면 버퍼 설정.
	swapChainDesc.BufferCount = 1;

	// 후면 버퍼의 너비와 높이 설정.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// 후면 버퍼에 일반적인 32비트 표면 설정.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// 후면 버퍼의 재생률 설정.
		if (m_vsync_enabled)
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
		}
		else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

	// 후면 버퍼의 용도 설정.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 렌더링할 윈도우 핸들 설정.
	swapChainDesc.OutputWindow = hwnd;

	// multisampling은 끈다.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// 풀스크린 or 창 모든 설정.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// 스캔 라인과 스캐일링을 unspecified로 설정.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 출력된 이후 후면 버퍼를 비운다.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 추가 플래그 사용하지 않음.
	swapChainDesc.Flags = 0;

		// feature 레벨을 DirectX 11로 설정.
		featureLevel = D3D_FEATURE_LEVEL_11_0;

		// 스왑 체인, Direct3D device, Direct3D device context 생성.
		result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
			D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}

		// 후면 버퍼의 포인터를 얻는다.
		result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// 후면 버퍼 포인터로 렌더 타겟 뷰 생성.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// 후면 버퍼 포인터 해제. 더 이상 필요하지 않다.
	backBufferPtr->Release();
	backBufferPtr = 0;

		// 깊이 버퍼의 description을 초기화 한다.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// 깊이 버퍼의 description에 각 값들을 설정.
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

		// 만들어진 description을 이용하여 깊이 버퍼에 대한 텍스쳐 생성.
		result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

		// 스텐실 상태 description 초기화.
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// 스텐실 상태 description 설정.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// 픽셀이 앞면(3차원 공간에서 다각형은 앞면, 뒷면의 그려짐이 다르다.)
	// 일 경우 스텐실 동작.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 픽셀이 뒷면일 경우 스텐실 동작.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// 깊이 스텐실 상태 생성.
		result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

		// 깊이 스텐실 상태 설정.
		m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

		// 깊이 스텐실 뷰 초기화.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// 깊이 스텐실 뷰 description 설정.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// 깊이 스텐실 뷰 생성.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

		// 렌더 타겟 뷰와 깊이 스텐실 버퍼를 출력 렌더 파이프라인에 바인딩 한다.
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

		// 어떻게 그리고 어떤 다각형들이 그려질 지를 결정하는 래스터 description 설정.
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

	// 채워진 description로부터 래스터라이저 상태 생성.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// 래스터라이저 설정.
	m_deviceContext->RSSetState(m_rasterState);

		// 렌더링을 위한 뷰포트 설정.
		viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// 뷰포트 생성
	m_deviceContext->RSSetViewports(1, &viewport);


		// 투영 행렬 설정.
		fieldOfView = (float)D3DX_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// 3D 렌더링을 위한 투영 행렬 생성.
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

		// 월드 행렬을 단위 행렬로 초기화
		D3DXMatrixIdentity(&m_worldMatrix);

		// 2D 렌더링을 위한 직교 투영 행렬 생성.
		D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}

void D3DClass::Shutdown()
{
	// 정리하기 전에 창모드로 설정, 그렇지 않으면 스왑 체인 해제시 예외가 발생한다.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	// 버퍼를 비울 색상 지정.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// 후면 버퍼 비움.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// 깊이 버퍼 비움.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void D3DClass::EndScene()
{
	// 렌더링이 완료되면 후면 버퍼를 화면에 출력.
	if (m_vsync_enabled)
	{
		// 화면 재생률 고정.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// 가능한 빠르게 출력.
		m_swapChain->Present(0, 0);
	}

	return;
}

ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

void D3DClass::GetProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(D3DXMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(D3DXMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}
