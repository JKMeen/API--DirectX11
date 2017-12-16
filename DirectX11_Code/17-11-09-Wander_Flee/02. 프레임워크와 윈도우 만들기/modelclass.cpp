////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"


ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	//m_Texture = 0;

	Model = new ModelType;

	Model->name = "Object";
	Model->location = new vec3d();
	Model->velocity = new vec3d();
	Model->color = new vec3d();

	for (int i = 0; i < 3; i++){
		Model->location->v[i] = 0;
		Model->velocity->v[i] = 0;
		Model->color->v[i] = 255;
	}


	Model->radius = 1;
	Model->nPoints = 10;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
	delete Model;
}


// 정점 버퍼와 인덱스 버퍼의 초기화 함수를 호출
//bool ModelClass::Initialize(ID3D11Device* device, WCHAR* textureFilename)
bool ModelClass::Initialize(int nPoints, float radius, ID3D11Device* device)
{
	bool result;

	int m_vCount = nPoints * 3;
	int m_iCount = nPoints * 3;

	// Initialize the vertex and index buffers.
	// 정점 버퍼와 인덱스 버퍼를 초기화
	// float x, float y, int m_vCount, int m_iCount, float radius, int nPoints, float angle, ID3D11Device* device

	Model->m_vCount = m_vCount;
	Model->m_iCount = m_iCount;
	Model->radius = radius;
	Model->nPoints = nPoints;

	// 모델링 생성
	result = InitializeBuffers(Model->m_vCount, Model->m_iCount, Model->radius, Model->nPoints, device);
	if (!result)
	{
		return false;
	}

	/*
	// Load the texture for this model.
	//result = LoadTexture(device, textureFilename);
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}*/

	return true;
}



///////////////// 기능 함수 //////////////////////////////////////////

// 위치 설정
void ModelClass::Set_Position(vec3d loc){
	for (int i = 0; i < 3; i++){
		this->Model->location->v[i] += loc.v[i];
		if (this->Model->location->v[0] + Model->velocity->v[0] < -50 || this->Model->location->v[0] + Model->velocity->v[0] > 50){
			Model->velocity->v[0] *= -1;
		}
		if (this->Model->location->v[1] + Model->velocity->v[1] < -40 || this->Model->location->v[1] + Model->velocity->v[1] > 40){
			Model->velocity->v[1] *= -1;
		}
		
		if (this->Model->location->v[0] < -50)
			this->Model->location->v[0] = 50;
		if (this->Model->location->v[0] > 50)
			this->Model->location->v[0] = -50;
		if (this->Model->location->v[1] < -30)
			this->Model->location->v[1] = 30;
		if (this->Model->location->v[1] > 30)
			this->Model->location->v[1] = -30;
	}
}

// 속도 설정
void ModelClass::Set_Velocity(vec3d vel){
	for (int i = 0; i < 3; i++)
		Model->velocity->v[i] += vel.v[i];
}

// 색 설정
void ModelClass::Set_Color(vec3d col){
	for (int i = 0; i < 3; i++)
		this->Model->color->v[i] = col.v[i];
}


///////////////////////////////////////////////////////////////////


// 정점 버퍼와 인덱스 버퍼를 정리하는 함수를 호출
void ModelClass::Shutdown()
{
	// Release the model texture.
	//ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	return;
}


// GraphicsClass::Render 함수에서 호출 됨
// RenderBuffer 함수를 호출하여 정점 버퍼와 인덱스 버퍼를 그래픽 파이프라인에 넣어 컬러 셰이더가 이들을 그릴 수 있도록 함
void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	// 정점 버퍼와 인덱스 버퍼를 그래픽스 파이프라인에 넣어 화면에 그릴 준비를 함

	RenderBuffers(deviceContext);

	return;
}


// 해당 모델의 인덱스의 개수를 알려줌
// 컬러 셰이더에서 모델을 그리기 위해 이 정보가 필요
int ModelClass::GetIndexCount()
{
	return m_indexCount;
}


/*ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}*/


// 정점 버퍼와 인덱스 버퍼를 생성하는 작업을 제어함
// 데이터 파일로부터 모델의 정보를 읽어와 버퍼들을 만드는 일을 함
bool ModelClass::InitializeBuffers(int m_vCount, int m_iCount, float radius, int nPoints, ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	
	// Set the number of vertices in the vertex array.
	// 정점 배열의 길이를 설정
	m_vertexCount = m_vCount;

	// Set the number of indices in the index array.
	// 인덱스 배열의 길이를 설정
	m_indexCount = m_iCount;

	// Create the vertex array.
	// 정점 배열을 생성
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	// 인덱스 배열을 생성
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}


	// 정점/인덱스 배열에 삼각형의 각 점과 그 순서를 채워넣음
	// *이것을 그리기 위해 점들을 시계 방향으로 만들어야 함 
	// (반시계로 만들면 DirectX에서 이 모델은 반대편을 바라본다고 판단, backface culling에 의해 그려지지 않게 됨)
	// GPU에서 도형을 그리도록 할 때 이순서를 기억하는 것이 중요
	// 정점의 description을 작성하기 때문에 색상 역시 정해주게 됨

	// Load the vertex array with data.
	// 정점 배열에 값을 넣음

	float angle = 2.0 * 3.14159;
	float step = (2.0 * 3.14159) / nPoints;


	/*for (int i = 0; i < 3; i++){
		Model->location->v[i] += Model->velocity->v[i];
	}*/


	// 팔각형
	for (int i = 0; i < m_indexCount; i+=3){
		vertices[i + 0].position = D3DXVECTOR3(radius*cos(angle) + this->Model->location->v[0], radius*sin(angle) + this->Model->location->v[1], 1.0f);
		vertices[i + 1].position = D3DXVECTOR3(radius*cos(angle - step) + this->Model->location->v[0], radius*sin(angle - step) + this->Model->location->v[1], 1.0f);
		vertices[i + 2].position = D3DXVECTOR3(0.0f + this->Model->location->v[0], 0.0f + this->Model->location->v[1], 1.0f);

		vertices[i + 0].color = D3DXVECTOR4(Model->color->v[0], Model->color->v[1], Model->color->v[2], 1.0f);
		vertices[i + 1].color = D3DXVECTOR4(Model->color->v[0], Model->color->v[1], Model->color->v[2], 1.0f);
		vertices[i + 2].color = D3DXVECTOR4(Model->color->v[0], Model->color->v[1], Model->color->v[2], 1.0f);
		
		angle -= step;
	}

	// Load the index array with data.
	// 인덱스 배열에 값을 넣음
	for (int i = 0; i < m_indexCount; i++){
		indices[i] = i;
	}

	// 정점 배열과 인덱스 배열이 채워졌으므로 정점 버퍼와 인덱스 버퍼를 만듦
	// 버퍼에 대한 description을 작성하는데, ByteWidth(버퍼의 크기)와 BindFlags(버퍼의 타입)을 정확히 입력
	// 방금 만들었던 정점 배열과 인덱스 배열을 subresource 포인터에 연결
	// description과 subresource 포인터, D3D 디바이스의 CreateBuffer 함수를 사용하여 새 버퍼의 포인터를 받아옴
	
	// Set up the description of the static vertex buffer.
	// 정점 버퍼의 description을 작성
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	// 정점 데이터를 가리키는 보조 리소스 구조체를 작성
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	// 정점 버퍼를 생성
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	// 인덱스 버퍼의 description을 작성
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	// 인덱스 버퍼를 생성
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}


	// 정점 버퍼와 인덱스 버퍼를 만든 후 이미 값이 복사되어 필요가 없어진 정점 배열과 인덱스 배열을 제거

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


// InitializeBuffers 함수에서 만들었던 정점 버퍼와 인덱스 버퍼를 해제하는 일을 함
void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	// 인덱스 버퍼를 해제
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	// 정점 버퍼를 해제
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


// Render 함수에서 호출되며, 정점 버퍼와 인덱스 버퍼를 GPU의 어셈블러의 버퍼로서 활성화 시킴
// GPU가 활성호된 정점 버퍼를 가지게 되면 셰이더를 이용하여 버퍼의 내용을 그릴 수 있게 됨
// 이 정점을 삼각형이나 선분, 부채꼴 등 어떤 모양으로 그리게 될 지 정의함
// 튜토리얼에서는 어셈블러의 입력에 정점 버퍼와 인덱스 버퍼를 넣고 DirectX의 IASetPrimitiveTopology 함수를 사용하여 GPU에게 이 정점들을 그리도록 주문
void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	// 정점 버퍼의 단위와 오프셋을 설정
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	// input assembler에 정점 버퍼를 활성화하여 그려질 수 있게 함
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	// input assembler에 인덱스 버퍼를 활성화하여 그려질 수 있게 함
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	// 정점 버퍼로 그릴 기본형을 설정 (삼각형)
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

/*
bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
	bool result;


	// Create the texture object.
	m_Texture = new TextureClass;
	if(!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_Texture->Initialize(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}*/