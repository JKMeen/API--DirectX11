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


// ���� ���ۿ� �ε��� ������ �ʱ�ȭ �Լ��� ȣ��
//bool ModelClass::Initialize(ID3D11Device* device, WCHAR* textureFilename)
bool ModelClass::Initialize(int nPoints, float radius, ID3D11Device* device)
{
	bool result;

	int m_vCount = nPoints * 3;
	int m_iCount = nPoints * 3;

	// Initialize the vertex and index buffers.
	// ���� ���ۿ� �ε��� ���۸� �ʱ�ȭ
	// float x, float y, int m_vCount, int m_iCount, float radius, int nPoints, float angle, ID3D11Device* device

	Model->m_vCount = m_vCount;
	Model->m_iCount = m_iCount;
	Model->radius = radius;
	Model->nPoints = nPoints;

	// �𵨸� ����
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



///////////////// ��� �Լ� //////////////////////////////////////////

// ��ġ ����
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

// �ӵ� ����
void ModelClass::Set_Velocity(vec3d vel){
	for (int i = 0; i < 3; i++)
		Model->velocity->v[i] += vel.v[i];
}

// �� ����
void ModelClass::Set_Color(vec3d col){
	for (int i = 0; i < 3; i++)
		this->Model->color->v[i] = col.v[i];
}


///////////////////////////////////////////////////////////////////


// ���� ���ۿ� �ε��� ���۸� �����ϴ� �Լ��� ȣ��
void ModelClass::Shutdown()
{
	// Release the model texture.
	//ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	return;
}


// GraphicsClass::Render �Լ����� ȣ�� ��
// RenderBuffer �Լ��� ȣ���Ͽ� ���� ���ۿ� �ε��� ���۸� �׷��� ���������ο� �־� �÷� ���̴��� �̵��� �׸� �� �ֵ��� ��
void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	// ���� ���ۿ� �ε��� ���۸� �׷��Ƚ� ���������ο� �־� ȭ�鿡 �׸� �غ� ��

	RenderBuffers(deviceContext);

	return;
}


// �ش� ���� �ε����� ������ �˷���
// �÷� ���̴����� ���� �׸��� ���� �� ������ �ʿ�
int ModelClass::GetIndexCount()
{
	return m_indexCount;
}


/*ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}*/


// ���� ���ۿ� �ε��� ���۸� �����ϴ� �۾��� ������
// ������ ���Ϸκ��� ���� ������ �о�� ���۵��� ����� ���� ��
bool ModelClass::InitializeBuffers(int m_vCount, int m_iCount, float radius, int nPoints, ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	
	// Set the number of vertices in the vertex array.
	// ���� �迭�� ���̸� ����
	m_vertexCount = m_vCount;

	// Set the number of indices in the index array.
	// �ε��� �迭�� ���̸� ����
	m_indexCount = m_iCount;

	// Create the vertex array.
	// ���� �迭�� ����
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	// �ε��� �迭�� ����
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}


	// ����/�ε��� �迭�� �ﰢ���� �� ���� �� ������ ä������
	// *�̰��� �׸��� ���� ������ �ð� �������� ������ �� 
	// (�ݽð�� ����� DirectX���� �� ���� �ݴ����� �ٶ󺻴ٰ� �Ǵ�, backface culling�� ���� �׷����� �ʰ� ��)
	// GPU���� ������ �׸����� �� �� �̼����� ����ϴ� ���� �߿�
	// ������ description�� �ۼ��ϱ� ������ ���� ���� �����ְ� ��

	// Load the vertex array with data.
	// ���� �迭�� ���� ����

	float angle = 2.0 * 3.14159;
	float step = (2.0 * 3.14159) / nPoints;


	/*for (int i = 0; i < 3; i++){
		Model->location->v[i] += Model->velocity->v[i];
	}*/


	// �Ȱ���
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
	// �ε��� �迭�� ���� ����
	for (int i = 0; i < m_indexCount; i++){
		indices[i] = i;
	}

	// ���� �迭�� �ε��� �迭�� ä�������Ƿ� ���� ���ۿ� �ε��� ���۸� ����
	// ���ۿ� ���� description�� �ۼ��ϴµ�, ByteWidth(������ ũ��)�� BindFlags(������ Ÿ��)�� ��Ȯ�� �Է�
	// ��� ������� ���� �迭�� �ε��� �迭�� subresource �����Ϳ� ����
	// description�� subresource ������, D3D ����̽��� CreateBuffer �Լ��� ����Ͽ� �� ������ �����͸� �޾ƿ�
	
	// Set up the description of the static vertex buffer.
	// ���� ������ description�� �ۼ�
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	// ���� �����͸� ����Ű�� ���� ���ҽ� ����ü�� �ۼ�
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	// ���� ���۸� ����
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	// �ε��� ������ description�� �ۼ�
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	// �ε��� �����͸� ����Ű�� ���� ���ҽ� ����ü�� �ۼ�
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	// �ε��� ���۸� ����
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}


	// ���� ���ۿ� �ε��� ���۸� ���� �� �̹� ���� ����Ǿ� �ʿ䰡 ������ ���� �迭�� �ε��� �迭�� ����

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	// �����ǰ� ���� �Ҵ�� ���� ���ۿ� �ε��� ���۸� ����
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


// InitializeBuffers �Լ����� ������� ���� ���ۿ� �ε��� ���۸� �����ϴ� ���� ��
void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	// �ε��� ���۸� ����
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	// ���� ���۸� ����
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


// Render �Լ����� ȣ��Ǹ�, ���� ���ۿ� �ε��� ���۸� GPU�� ������� ���۷μ� Ȱ��ȭ ��Ŵ
// GPU�� Ȱ��ȣ�� ���� ���۸� ������ �Ǹ� ���̴��� �̿��Ͽ� ������ ������ �׸� �� �ְ� ��
// �� ������ �ﰢ���̳� ����, ��ä�� �� � ������� �׸��� �� �� ������
// Ʃ�丮�󿡼��� ������� �Է¿� ���� ���ۿ� �ε��� ���۸� �ְ� DirectX�� IASetPrimitiveTopology �Լ��� ����Ͽ� GPU���� �� �������� �׸����� �ֹ�
void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	// ���� ������ ������ �������� ����
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	// input assembler�� ���� ���۸� Ȱ��ȭ�Ͽ� �׷��� �� �ְ� ��
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	// input assembler�� �ε��� ���۸� Ȱ��ȭ�Ͽ� �׷��� �� �ְ� ��
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	// ���� ���۷� �׸� �⺻���� ���� (�ﰢ��)
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