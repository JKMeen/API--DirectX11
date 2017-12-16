////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

// ��Ŭ������ 3D �𵨵��� ������ �����е��� ĸ��ȭ�ϴ� Ŭ�����̴�


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include "vec3d.h"

/*
///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"*/


////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
private:
	// ���� ���ۿ��� ����� ������ ����ü
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 color;
	    //D3DXVECTOR2 texture;
	};

	

	struct ModelType{
		vec3d *location;
		vec3d *velocity;
		vec3d *color;
		
		int m_vCount;
		int m_iCount;
		float radius;
		int nPoints;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	// 3D ���� ���� ���ۿ� �ε��� ���۵��� �ʱ�ȭ�� ���� ������ ����
	//bool Initialize(float x, float y, float z, float v1, float v2, float v3, int m_vCount, int m_iCount, float radius, int nPoints, float r, float g, float b, ID3D11Device*);
	bool Initialize(int m_vCount, int m_iCount, float radius, int nPoints, ID3D11Device*);
	//bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*); // �׷��� ī�忡 �𵨵��� ���� ������ �ְ� �÷� ���̴��� �׸� �غ� ��

	int GetIndexCount();
	//ID3D11ShaderResourceView* GetTexture();

	/*void Set_Position(float x, float y, float z);
	void Set_Velocity(float v1, float v2, float v3);
	void Set_Color(float r, float g, float b);*/

	void Set_Position(vec3d loc);
	void Set_Velocity(vec3d vel);
	void Set_Color(vec3d col);



private:
	bool InitializeBuffers(int m_vCount, int m_iCount, float radius, int nPoints, ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	//bool LoadTexture(ID3D11Device*, WCHAR*);
	//void ReleaseTexture();

private:
	// ���� ���ۿ� �ε��� ����, �� ������ ũ�� ������ ������ �ִ� ����
	// DirectX 11�� ���۵��� �밳 �Ϲ����� ID3D11Buffer Ÿ���� �̿��ϸ�, ������ �� ���� description���� ����
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;

public:
	ModelType *Model;
	//TextureClass* m_Texture;
};

#endif