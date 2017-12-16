////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"

SystemClass::SystemClass()
{
	m_Input = 0;
	m_Graphics = 0;
}

SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;


	// 함수안으로 변수를 전달하기전에 화면 너비와 높이를 0으로 초기화한다.
	screenWidth = 0;
	screenHeight = 0;

	// windows api를 초기화한다.
	InitializeWindows(screenWidth, screenHeight);

	// input 객체 생성.  이 객체는 사용자로부터 키보드를 읽는 것을 처리하는데 사용될 것이다.
	m_Input = new InputClass;
	if (!m_Input)
	{
		return false;
	}

	// input 객체 초기화.
	m_Input->Initialize();

	// graphics 객체 생성.  어플리케이션을 위한 모든 그래픽을 렌더링하는 것을 처리한다.
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}

	// graphics 객체 초기화.
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result)
	{
		return false;
	}

	return true;
}

void SystemClass::Shutdown()
{
	// graphics 객체 해제.
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// input 해제.
	if (m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// 윈도우 shutdown.
	ShutdownWindows();

	return;
}

void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// 메시지 구조체 초기화.
	ZeroMemory(&msg, sizeof(MSG));

	// 윈도우나 사용자로부터 종료 메시지가 있을때까지 돈다.
	done = false;
	while (!done)
	{
		// 윈도우 메시지를 처리한다.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// 윈도우가 어플리케이션 종료를 보내면 루프를 종료한다.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// 종료 메시지 외 에는 frame 처리를 한다.
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}

	}

	return;

}

bool SystemClass::Frame()
{
	bool result;

	if (m_Input->IsKeyDown(VK_UP))
		m_Graphics->angle_speed += 0.005f;

	if (m_Input->IsKeyDown(VK_DOWN))
		m_Graphics->angle_speed -= 0.005f;

	if (m_Input->IsKeyDown('1'))
		m_Graphics->Axis = 1;
	if (m_Input->IsKeyDown('2'))
		m_Graphics->Axis = 2;
	if (m_Input->IsKeyDown('3'))
		m_Graphics->Axis = 3;
	if (m_Input->IsKeyDown('4'))
		m_Graphics->Axis = 4;

	if (m_Input->IsKeyDown('X'))
		m_Graphics->Axis_X = !m_Graphics->Axis_X;

	if (m_Input->IsKeyDown('Y'))
		m_Graphics->Axis_Y = !m_Graphics->Axis_Y;

	if (m_Input->IsKeyDown('Z'))
		m_Graphics->Axis_Z = !m_Graphics->Axis_Z;

	// 사용자가 Esc키를 눌렀고 프로그램을 종료하기를 원하는지 확인.
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	// graphics 객체의 Frame() 함수 호출.
	result = m_Graphics->Frame();
	if (!result)
	{
		return false;
	}

	return true;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// 키보드상의 키가 눌렸는지를 확인한다.
	case WM_KEYDOWN:
	{
		// 만약 키가 눌렸다면 input객체에게 key를 전달한다. 그래서 상태를 기록한다.
		m_Input->KeyDown((unsigned int)wparam);
		return 0;
	}

	// 키보드상의 키가 떼어졌는지를 확인한다.
	case WM_KEYUP:
	{
		// 만약 키가 떼어졌다면 input객체에게 전달한다. 그래서 상태를 지운다.
		m_Input->KeyUp((unsigned int)wparam);
		return 0;
	}

	// 그 외 다른 모든 메시지는 우리의 어플리케이션은 사용하지 않기 때문에 default 메시지 처리기로 전달한다.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// 외부 포인터로 현재 객체의 주소를 담습니다.
	ApplicationHandle = this;

	// 현재 어플리케이션의 인스턴스를 얻습니다.
	m_hinstance = GetModuleHandle(NULL);

	// 어플리케이션 이름 설정.
	m_applicationName = L"Engine";

	// 윈도우 클래스를 기본으로 설정.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//배경색
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// 윈도우 클래스 등록.
	RegisterClassEx(&wc);

	// 사용자 환경에 맞춘 화면크기 결정.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// 전체 화면 여부에 따른 화면 관련 설정.
	if (FULL_SCREEN)
	{
		// 만약 전체화면모드이면 화면을 최대사이즈로 하고 32비트 컬러로 한다.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// 디스플레이 설정을 전체화면으로 변경.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// 창 위치를 왼쪽 최상단에 위치.
		posX = posY = 0;
	}
	else
	{
		// 창모드일 겨우 800x600으로 설정.
		screenWidth = 800;
		screenHeight = 600;

		// 창이 화면 중앙에 위치하도록 위치 설정.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// 윈도우(창)을 설정에 맞게 생성하고 해당 윈도우에 대한 핸들을 얻는다.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// 윈도우(창)을 가장 위쪽(위일수록 보이며 아래일수록 다른 화면들에 가려진다.)에 둔다.
	// 그리고 활성화시킨다. (현재 작업중인 윈도우로)
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// 마우스 커서를 숨긴다.
	ShowCursor(false);

	return;
}

void SystemClass::ShutdownWindows()
{
	// 마우스 커서를 보인다.
	ShowCursor(true);

	// 전체화면모드이면 화면 설정을 변경한다.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// 윈도우(창) 제거.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// 어플리케이션 인스턴스 제거.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// 현재 객체로의 포인터 해제.
	ApplicationHandle = NULL;

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// 윈도우를 제거중인지 확인한다.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// 윈도우를 닫는중인지 확인한다.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// 그 외 모든 메시지는 system 클래스의 메시지처리기로 넘긴다.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}
