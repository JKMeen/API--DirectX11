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


	// �Լ������� ������ �����ϱ����� ȭ�� �ʺ�� ���̸� 0���� �ʱ�ȭ�Ѵ�.
	screenWidth = 0;
	screenHeight = 0;

	// windows api�� �ʱ�ȭ�Ѵ�.
	InitializeWindows(screenWidth, screenHeight);

	// input ��ü ����.  �� ��ü�� ����ڷκ��� Ű���带 �д� ���� ó���ϴµ� ���� ���̴�.
	m_Input = new InputClass;
	if (!m_Input)
	{
		return false;
	}

	// input ��ü �ʱ�ȭ.
	m_Input->Initialize();

	// graphics ��ü ����.  ���ø����̼��� ���� ��� �׷����� �������ϴ� ���� ó���Ѵ�.
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}

	// graphics ��ü �ʱ�ȭ.
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result)
	{
		return false;
	}

	return true;
}

void SystemClass::Shutdown()
{
	// graphics ��ü ����.
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// input ����.
	if (m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// ������ shutdown.
	ShutdownWindows();

	return;
}

void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// �޽��� ����ü �ʱ�ȭ.
	ZeroMemory(&msg, sizeof(MSG));

	// �����쳪 ����ڷκ��� ���� �޽����� ���������� ����.
	done = false;
	while (!done)
	{
		// ������ �޽����� ó���Ѵ�.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// �����찡 ���ø����̼� ���Ḧ ������ ������ �����Ѵ�.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// ���� �޽��� �� ���� frame ó���� �Ѵ�.
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

	// ����ڰ� EscŰ�� ������ ���α׷��� �����ϱ⸦ ���ϴ��� Ȯ��.
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	// graphics ��ü�� Frame() �Լ� ȣ��.
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
		// Ű������� Ű�� ���ȴ����� Ȯ���Ѵ�.
	case WM_KEYDOWN:
	{
		// ���� Ű�� ���ȴٸ� input��ü���� key�� �����Ѵ�. �׷��� ���¸� ����Ѵ�.
		m_Input->KeyDown((unsigned int)wparam);
		return 0;
	}

	// Ű������� Ű�� ������������ Ȯ���Ѵ�.
	case WM_KEYUP:
	{
		// ���� Ű�� �������ٸ� input��ü���� �����Ѵ�. �׷��� ���¸� �����.
		m_Input->KeyUp((unsigned int)wparam);
		return 0;
	}

	// �� �� �ٸ� ��� �޽����� �츮�� ���ø����̼��� ������� �ʱ� ������ default �޽��� ó����� �����Ѵ�.
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


	// �ܺ� �����ͷ� ���� ��ü�� �ּҸ� ����ϴ�.
	ApplicationHandle = this;

	// ���� ���ø����̼��� �ν��Ͻ��� ����ϴ�.
	m_hinstance = GetModuleHandle(NULL);

	// ���ø����̼� �̸� ����.
	m_applicationName = L"Engine";

	// ������ Ŭ������ �⺻���� ����.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//����
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// ������ Ŭ���� ���.
	RegisterClassEx(&wc);

	// ����� ȯ�濡 ���� ȭ��ũ�� ����.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// ��ü ȭ�� ���ο� ���� ȭ�� ���� ����.
	if (FULL_SCREEN)
	{
		// ���� ��üȭ�����̸� ȭ���� �ִ������� �ϰ� 32��Ʈ �÷��� �Ѵ�.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// ���÷��� ������ ��üȭ������ ����.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// â ��ġ�� ���� �ֻ�ܿ� ��ġ.
		posX = posY = 0;
	}
	else
	{
		// â����� �ܿ� 800x600���� ����.
		screenWidth = 800;
		screenHeight = 600;

		// â�� ȭ�� �߾ӿ� ��ġ�ϵ��� ��ġ ����.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// ������(â)�� ������ �°� �����ϰ� �ش� �����쿡 ���� �ڵ��� ��´�.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// ������(â)�� ���� ����(���ϼ��� ���̸� �Ʒ��ϼ��� �ٸ� ȭ��鿡 ��������.)�� �д�.
	// �׸��� Ȱ��ȭ��Ų��. (���� �۾����� �������)
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// ���콺 Ŀ���� �����.
	ShowCursor(false);

	return;
}

void SystemClass::ShutdownWindows()
{
	// ���콺 Ŀ���� ���δ�.
	ShowCursor(true);

	// ��üȭ�����̸� ȭ�� ������ �����Ѵ�.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// ������(â) ����.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// ���ø����̼� �ν��Ͻ� ����.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// ���� ��ü���� ������ ����.
	ApplicationHandle = NULL;

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// �����츦 ���������� Ȯ���Ѵ�.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// �����츦 �ݴ������� Ȯ���Ѵ�.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// �� �� ��� �޽����� system Ŭ������ �޽���ó����� �ѱ��.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}
