////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
// 자주 사용되지 않는 API들을 담고 있는 Win32헤더를 포함하지 않으므로
// 빌드 과정의 속도를 높임
#define WIN32_LEAN_AND_MEAN


//////////////
// INCLUDES //
//////////////
// 윈도우 생성/제거하는 함수들을 호출할 수 있음
#include <windows.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
// system 클래스에서 사용하기 위해 프레임워크의 두 클래스의 헤더를 포함
#include "inputclass.h"
#include "graphicsclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: SystemClass
////////////////////////////////////////////////////////////////////////////////
class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();
	void Draw();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input;
	GraphicsClass* m_Graphics;

	float Pos_x;
	float Pos_y;
	float Pos_z;
};


/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/////////////
// GLOBALS //
/////////////
static SystemClass* ApplicationHandle = 0;


#endif