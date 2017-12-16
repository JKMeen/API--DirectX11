////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
// ���� ������ �ʴ� API���� ��� �ִ� Win32����� �������� �����Ƿ�
// ���� ������ �ӵ��� ����
#define WIN32_LEAN_AND_MEAN


//////////////
// INCLUDES //
//////////////
// ������ ����/�����ϴ� �Լ����� ȣ���� �� ����
#include <windows.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
// system Ŭ�������� ����ϱ� ���� �����ӿ�ũ�� �� Ŭ������ ����� ����
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