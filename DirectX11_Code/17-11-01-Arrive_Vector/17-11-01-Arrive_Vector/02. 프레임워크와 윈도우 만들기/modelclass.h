////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

// 모델클래스는 3D 모델들의 복잡한 기하학들을 캡슐화하는 클래스이다


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
	// 정점 버퍼에서 사용할 정점의 구조체
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

	// 3D 모델의 정점 버퍼와 인덱스 버퍼들의 초기화와 종료 과정을 제어
	//bool Initialize(float x, float y, float z, float v1, float v2, float v3, int m_vCount, int m_iCount, float radius, int nPoints, float r, float g, float b, ID3D11Device*);
	bool Initialize(int m_vCount, int m_iCount, float radius, int nPoints, ID3D11Device*);
	//bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*); // 그래픽 카드에 모델들의 기하 정보를 넣고 컬러 셰이더로 그릴 준비를 함

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
	// 정점 버퍼와 인덱스 버퍼, 각 버퍼의 크기 정보를 가지고 있는 변수
	// DirectX 11의 버퍼들은 대개 일반적인 ID3D11Buffer 타입을 이용하며, 생성할 때 버퍼 description으로 구분
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;

public:
	ModelType *Model;
	//TextureClass* m_Texture;
};

#endif