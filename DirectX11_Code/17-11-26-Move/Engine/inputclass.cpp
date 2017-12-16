////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "inputclass.h"


InputClass::InputClass()
{
}


InputClass::InputClass(const InputClass& other)
{
}


InputClass::~InputClass()
{
}


void InputClass::Initialize()
{
	int i;


	// 모든 키들을 눌리지 않은 상태로 초기화한다.
	for (i = 0; i<256; i++)
	{
		m_keys[i] = false;
	}

	return;
}


void InputClass::KeyDown(unsigned int input)
{
	// 키가 눌릴 경우 키배열에 상태를 저장한다.
	m_keys[input] = true;
	return;
}


void InputClass::KeyUp(unsigned int input)
{
	// 키가 떼어질 경우 키배열을 비활성으로 되돌린다.
	m_keys[input] = false;
	return;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// 키의 상태가 눌렸는지 눌리지 않았는지를 반환한다.( 1:눌림, 0:안 눌림)
	return m_keys[key];
}


