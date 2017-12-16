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

// 객체 초기화
// DirectX 11의 전체 Direct3D 설정이 일어나는 곳
// SystemClass에서 만든 윈도우의 너비와 높이를
// Direct3D에 이용하여 동일한 크기의 영역을 초기화하고 이용
// hWnd 변수는 만들어진 윈도우에 대한 핸들
// sereenDepth, screenNear : 윈도우에 그려질 3D 환경에서의 깊이 값
// vsync : Direct3D 렌더링을 모니터의 새로고침 비율에 맞출 것인지, 빠르게 다시 그릴 것인지 지정
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
	// vsync(수직동기화) 설정을 저장
	m_vsync_enabled = vsync;

	// Create a DirectX graphics interface factory.
	// DirectX 그래픽 인터페이스 팩토리를 만듦
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if(FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	// 팩토리 객체를 사용하여 첫 번째 그래픽 카드 인터페이스에 대한 어댑터를 만듦
	result = factory->EnumAdapters(0, &adapter);
	if(FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	// 출력 (모니터)에 대한 첫 번째 어댑터를 나열
	result = adapter->EnumOutputs(0, &adapterOutput);
	if(FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	// DXGI_FORMAT_R8G8B8A8_UNORM 모니터 출력 디스플레이 포멧에 맞는 모드의 개수를 구함
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	// 가능한 모든 모니터와 그래픽 카드 조합을 저장할 리스트를 생성
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	// 디스플레이 모드에 대한 리스트 구조를 채워넣음
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	// 모든 디스플레이 모드에 대해 화면 너비/높이에 맞는 디스플레이를 찾고,
	// 적합한 것을 찾으면 모니터의 새로고침 비율의 분모와 분자 값을 저장
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
	// 어댑터(그래픽카드)의 description을 가져옵니다.
	result = adapter->GetDesc(&adapterDesc);
	if(FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	// 현재 그래픽카드의 메모리 용량을 메가바이트 단위로 저장
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	// 그래픽카드의 이름을 char형 문자열 배열로 바꾼 뒤 저장
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if(error != 0)
	{
		return false;
	}


	/////////////////////////////////////////////////////
	// 저장된 새로고침 비율의 분자/분모 값과 그래픽카드의 정보가 있기 때문에
	// 정보를 얻기 위해 사용한 구조체들과 인터페이스들을 반납
	////////////////////////////////////////////////////////////////////////////

	// Release the display mode list.
	// 디스플레이 모드 리스트의 할당을 해제
	delete [] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	// 출력 어댑터를 할당 해제
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	// 어댑터를 할당 해제
	adapter->Release();
	adapter = 0;

	// Release the factory.
	// 팩토리 객체를 할당 해제
	factory->Release();
	factory = 0;


	///////////////////////////////////////////////////////////////////////////
	// 새로고침 비율을 얻었으므로 DirectX의 초기화를 시작
	// 가장 먼저 스왑 체인의 description 구조체를 채워 넣음
	// 스왑 체인은 프론트버퍼와 백버퍼로 이루어져 있는 실제로 렌더링을 하면 쓰는 곳
	// 하나의 백버퍼만을 사용하여 그 위에 그린 뒤 프론트 버퍼와 바꿔치기 함 : 스왑 체인
	////////////////////////////////////////////////////////////////////////////

	// Initialize the swap chain description.
	// 스왑 체인 description을 초기화 함
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	// 하나의 백버퍼만을 사용
    swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	// 백버퍼의 너비와 높이를 설정
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
	// 백버퍼로 일반적인 32bit의 서페이스를 지정
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	/////////////////////////////////////////////////////////////////////////
	// 스왑 체인 구조체 채우기 후 새로고침 비율을 설정
	// 새로고침 비율은 초당 몇 장의 백버퍼를 그려 프론트 버퍼와 바꿔치기하는지를 나타내는 숫자
	// graphicsclass.h의 vsync 변수가 true로 설정되어 있으면 시스템에서 정한
	// 새로고침 비율로 고정(ex: 60hz), 초당 60번 화면을 그림
	// vsync 변수가 false일 경우 최대한 많은 화면을 그려내려고 함 > 잔상을 남김
	////////////////////////////////////////////////////////////////////////////

	// Set the refresh rate of the back buffer.
	// 백버퍼의 새로고침 비율을 설정
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
	// 백버퍼의 용도를 설정
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	// 렌더링이 이루어질 윈도우의 핸들을 설정
    swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	// 멀티샘플링을 끔
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	// 윈도우 모드 또는 풀스크린 모드를 설정
	if(fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	// 스캔라인의 정렬과 스캔라이닝을 지정되지 않음(unspecified)으로 설정
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	// 출력된 이후의 백버퍼의 내용을 버리도록 함
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	// 추가 옵션 플래그를 사용하지 않음
	swapChainDesc.Flags = 0;


	//////////////////////////////////////////////////////////////////////////
	// 스왑 체인 description 구조체를 채움과 더불어 피쳐 레벨이라는 변수도 설정
	// 이 변수는 우리가 어느 버전의 DirectX를 사용할 것인지 알려줌
	// 버젼을 11.0으로 설정하고, 다른 버젼도 설정 가능
	////////////////////////////////////////////////////////////////////////////

	// Set the feature level to DirectX 11.
	// 피쳐 레벨을 DirectX 11로 설정
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	////////////////////////////////////////////////////////////////////////////
	// Direct3D 디바이스와 Direct3D디바이스 컨텐스트는 모든 Direct3D 기능의 인터페이스가 되기 때문에 중요함
	// 디바이스와 디바이스 컨텍스트를 사용하여 대부분의 작업을 수행
	// 유저가 DirectX11을 지원하는 그래픽카드를 가지고 있지 않다면
	// 디바이스와 디바이스 컨텍스트를 생성하는 함수가 실패할 것임,
	// 따라서 DirectX 11의 기능을 테스트해보고는 싶지만 그래픽카드가 DirectX 11을
	// 지원하지 않는 경우 이 부분에서 D3D_DRIVER_TYPE_HARDWARE를 D3D_DRIVER_TYPE_REFERENCE로 바꾸어서
	// 렌더링을 CPU에서 처리하게 할 수 있음
	// 속도는 1/1000 정도로 느리지만 DirectX11을 지원하지 않는 그래픽 카드를 가진 사람들에게 좋은 기능임
	////////////////////////////////////////////////////////////////////////////

	// Create the swap chain, Direct3D device, and Direct3D device context.
	// 스왑 체인, Direct3D 디바이스, Direct3D 디바이스 컨텍스트를 생성
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, 
										   D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if(FAILED(result))
	{
		return false;
	}


	////////////////////////////////////////////////////////////////////////////
	// 기본 그래픽 카드로 생성하지 않고 모든 비디오 카드들을 대조하여 가장 잘 맞는 그래픽 카드로 생성 할 수 있도록 함
	// 스왑 체인이 생겼으므로 백버퍼를 만들고 그 포인터를 스왑 체인에 연결시켜주어야 함
	// CreateRenderTargetView 함수를 사용하여 백버퍼를 스왑 체인에 연결함
	////////////////////////////////////////////////////////////////////////////

	// Get the pointer to the back buffer.
	// 백버퍼의 포인터를 가져옴
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if(FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	// 백버퍼의 포인터로 렌더타겟 뷰를 생성
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if(FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	// 백버퍼 포인터를 더 이상 사용하지 않으므로 할당 해제
	backBufferPtr->Release();
	backBufferPtr = 0;


	////////////////////////////////////////////////////////////////////////////
	// 깊이 버퍼의 description 구조체를 작성해야 함
	// 깊이 버퍼를 만들어야 3D 공간에서 폴리곤들이 올바르게 그려짐
	// 또한 동시에 스텐실 버퍼도 이 깊이 버퍼에 열결함
	// 스텐실 버퍼는 모션 블러나 볼륨있는 그림자 등의 효과를 나타낼 때 사용
	////////////////////////////////////////////////////////////////////////////

	// Initialize the description of the depth buffer.
	// 깊이 버퍼의 description을 초기화 함
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	// 깊이 버퍼의 dscription을 작성함
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
	// 이 정보를 이용하여 깊이/스텐실 버퍼의 description을 만들 수 있게 됨
	// CreateTexture2D 함수를 이용함으로 버퍼는 2D 텍스쳐라는 것을 알 수 있음
	// 폴리곤들이 정렬되고 레스터화된 이후에는 2D좌표의 픽셀들이 되기 때문임
	////////////////////////////////////////////////////////////////////////////

	// Create the texture for the depth buffer using the filled out description.
	// description을 사용하여 깊이 버퍼의 텍스쳐를 생성함
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if(FAILED(result))
	{
		return false;
	}


	////////////////////////////////////////////////////////////////////////////
	// 깊이-스텐실 description을 작성할 것이며,
	// Direct3D에서 각 픽셀에 어떤 깊이 테스트를 할 것인지 정할 수 있게 해줌
	////////////////////////////////////////////////////////////////////////////

	// Initialize the description of the stencil state.
	// 스텐실 상태의 description을 초기화 함
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	// 스텐실 상태의 description을 작성함
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	// 픽셀이 앞에 있을 경우 변환
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	// 픽셀이 뒤에 있을 경우 변환
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	////////////////////////////////////////////////////////////////////////////
	// description을 토대로 깊이-스텐실 상태 변수를 만듦
	////////////////////////////////////////////////////////////////////////////

	// Create the depth stencil state.
	// 깊이-스텐실 상태를 생성
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if(FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	// 깊이-스텐실 상태를 설정
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);


	////////////////////////////////////////////////////////////////////////////
	// 이제 깊이-스텐실 버퍼의 뷰에 대한 description을 만듦
	// 이 작업을 해야 Direct3D에서 깊이 버퍼를 깊이-스텐실 텍스쳐로 인식함
	// 이 구조체를 채우고 난 후에 CreateDepthStencilView 함수를 호출하여 생성함
	////////////////////////////////////////////////////////////////////////////

	// Initialize the depth stencil view.
	// 깊이-스텐실 뷰의 description을 초기화 함
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	// 깊이-스텐실 뷰의 description을 작성함
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	// 깊이-스텐실 뷰를 생성함
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if(FAILED(result))
	{
		return false;
	}


	////////////////////////////////////////////////////////////////////////////
	// OMSetRenderTarget 함수를 호출할 수 있게 됨
	// OMSetRenderTarget : 렌더타겟뷰와 깊이-스텐실 뷰를 출력 렌더링 파이프라인에 바인딩 시켜줌
	// 파이프라인을 이용한 렌더링이 수행될 때 우리가 만들었던 백버퍼에 그려지게 됨
	// 백버퍼에 그려진 것을 프론트 버퍼와 바꿔치기하여 유저의 모니터에 출력함
	////////////////////////////////////////////////////////////////////////////

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	// 렌더타겟 뷰와 깊이-스텐실 버퍼를 각각 출력 파이프라인에 바인딩함
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);


	////////////////////////////////////////////////////////////////////////////
	// 렌더 타겟이 설정된 뒤 추가적인 기능들을 설정할 수 있음
	// 첫번째로 만들게 되는 것은 레스터화기 상태(rasterizer state)
	// 이것은 도형이 어떻게 그려지는지에 대한 제어를 가질 수 있게됨
	// 이것을 이용하여 화면을 와이어프레임 모드로 그리거나 도형의 앞뒷면을 모두 그리도록 할 수 있음
	// 기본적으로 DirectX에서 설정되고 돌아가는 레스터화기 상태가 있지만 새로 만들지 않으면 제어권이 없음
	////////////////////////////////////////////////////////////////////////////

	// Setup the raster description which will determine how and what polygons will be drawn.
	// 어떤 도형을 어떻게 그릴 것인지 결정하는 레스터화기 description을 작성
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
	// 작성한 description으로부터 레스터화기 상태를 생성
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if(FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	// 레스터화기 상태를 설정
	m_deviceContext->RSSetState(m_rasterState);
	

	////////////////////////////////////////////////////////////////////////////
	// 뷰포트로 있어야 렌더타겟 공간에서 클리핑을 수행할 수 있음
	// 우선 이것을 윈도우 크기와 동일하게 설정함
	////////////////////////////////////////////////////////////////////////////

	// Setup the viewport for rendering.
	// 렌더링을 위한 뷰포트를 설정
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

	// Create the viewport.
	// 뷰포트를 생성
    m_deviceContext->RSSetViewports(1, &viewport);


	////////////////////////////////////////////////////////////////////////////
	// 이제 투영행렬(projection matrix)을 생성할 차례
	// 투영 행렬은 3D의 화면을 앞서 만들었던 2D 뷰포트 공간으로 변환하도록 해줌
	// 이 행렬의 복사본을 만들어 셰이더에서 사용할 수 있도록 해야 함
	////////////////////////////////////////////////////////////////////////////

	// Setup the projection matrix.
	// 투영 행렬을 설정함
	fieldOfView = (float)D3DX_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	// 3D 렌더링을 위한 투영 행렬을 생성함
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);


	////////////////////////////////////////////////////////////////////////////
	// 월드 행렬이라는 또다른 행렬을 만들어야 함
	// 이 행렬은 우리의 오브젝트들을 3D 세계의 좌표로 변환하는데 사용됨
	// 또한 3차원상에서의 회전/이동/크기 변환에서도 사용 됨
	// 복사본을 만들어 셰이더에서 사용할 수 있게 함
	////////////////////////////////////////////////////////////////////////////

    // Initialize the world matrix to the identity matrix.
	// 월드 행렬을 단위 행렬로 초기화함
    D3DXMatrixIdentity(&m_worldMatrix);


	////////////////////////////////////////////////////////////////////////////
	// 월드 행렬은 뷰 행렬을 생성하는 곳이 됨, 뷰 행렬은 현재 장면에서 우리가
	// 어디로 보고 있는가를 계산하는 데 이용됨
	// 그런 일을 하는 것이라면 쉽게 카메라를 생각해낼 수 있음
	// Initalize 함수에서 마지막으로 해야할 것은 정사영 행렬 만들기
	// 정사영 행렬은 3D객체가 아니라 UI와 같은 2D의 요소들을 그리기 위해 사용됨
	////////////////////////////////////////////////////////////////////////////

	// Create an orthographic projection matrix for 2D rendering.
	// 2D 렌더링에 사용될 정사영 행렬을 생성함
	D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);

    return true;
}

// 객체 정리 (종료 시 사용)
// Initalize 함수에서 사용했던 포인터들을 정리하고 반납하는 일을 함
// 이 작업을 하기 전에 스왑 체인을 윈도우 모드로 바꾸는 함수를 호출함
// 이 작업이 이루어지지 않는다면 풀스크린 상태에서 스왑 체인의 해제가 일어날 때
// 몇몇 예외가 발생하게 됨
// 따라서 그런 예외를 피하기 위해 Direct3D를 종료하기 전에 언제나 스왑 체인을 윈도우 모드로 바꿔주어야 함
void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	// 종료하기 전에 이렇게 윈도우 모드를 바꾸지 않으면 스왑체인을 할당 해제할 때 예외가 발생함
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


// 새로운 매 프레임마다 3D 화면을 그리기 시작할 때 호출됨
// 버퍼를 빈 값으로 초기화하고 렌더링이 이루어지도록 준비함
void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	// 버퍼를 어떤 생상으로 지울 것인지 설정
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	// 백버퍼의 내용을 지움
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
    
	// Clear the depth buffer.
	// 깊이 버퍼를 지움
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


// 매 프레임의 마지막에 스왑 체인에게 백버퍼에 그린 3D 화면을 표시하도록 하게 해줌
void D3DClass::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	// 렌더링이 완료되었으므로 백버퍼의 내용을 화면에 표시함
	if(m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		// 새로고침 비율을 고정
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		// 가능한 한 빠르게 표시함
		m_swapChain->Present(0, 0);
	}

	return;
}


// Direct3D 디바이스의 포인터를 가져옴
ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}

// Direct3D 디바이스 컨텍스트의 포인터를 가져옴
ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

// 투영 행렬을 반환
void D3DClass::GetProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}

// 월드 행렬을 반환
void D3DClass::GetWorldMatrix(D3DXMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}

// 정사영 행렬을 반환
void D3DClass::GetOrthoMatrix(D3DXMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

// 그래픽카드의 이름과 사용가능한 메모리의 양을 반환
// 그래픽 카드의 이름과 메모리의 크기를 아는 것은 서로 다른 설정에서 디버깅하는데 도움이 됨
void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}