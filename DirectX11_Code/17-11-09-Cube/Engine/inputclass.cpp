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


	// ��� Ű���� ������ ���� ���·� �ʱ�ȭ�Ѵ�.
	for (i = 0; i<256; i++)
	{
		m_keys[i] = false;
	}

	return;
}


void InputClass::KeyDown(unsigned int input)
{
	// Ű�� ���� ��� Ű�迭�� ���¸� �����Ѵ�.
	m_keys[input] = true;
	return;
}


void InputClass::KeyUp(unsigned int input)
{
	// Ű�� ������ ��� Ű�迭�� ��Ȱ������ �ǵ�����.
	m_keys[input] = false;
	return;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// Ű�� ���°� ���ȴ��� ������ �ʾҴ����� ��ȯ�Ѵ�.( 1:����, 0:�� ����)
	return m_keys[key];
}


