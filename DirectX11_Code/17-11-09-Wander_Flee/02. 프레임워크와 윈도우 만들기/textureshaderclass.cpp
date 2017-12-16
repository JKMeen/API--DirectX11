////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureshaderclass.h"


TextureShaderClass::TextureShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
}


TextureShaderClass::TextureShaderClass(const TextureShaderClass& other)
{
}


TextureShaderClass::~TextureShaderClass()
{
}


// ���̴��� �ʱ�ȭ�� �����ϴ� �Լ��� ȣ���ϸ�, texture.vs, texture.ps�� HLSL���̴� ������ �̸��� �Ѱ���
bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	// ���� ���̴��� �ȼ� ���̴��� �ʱ�ȭ ��
	result = InitializeShader(device, hwnd, L"../Engine/texture.vs", L"../Engine/texture.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


// ���̴��� �����ϴ� �Լ��� ȣ��
void TextureShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	// ���� ���̴��� �ȼ� ���̴� �� �׿� ���õ� �͵��� ��ȯ��
	ShutdownShader();

	return;
}


// Render �Լ������� �켱 SetshaderParameters �Լ��� ����Ͽ� ���̴����� ���� ���ڵ��� ������
// ���ڵ��� �����Ǹ� RenderShader �Լ��� ȣ���Ͽ� HLSL ���̴��� �̿��Ͽ� �� �׸��⸦ ������
bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
								D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	// �������� ����� ���̴��� ���ڸ� �Է�
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	// ���̴��� �̿��Ͽ� �غ�� ���۸� �׸�
	RenderShader(deviceContext, indexCount);

	return true;
}


// ������ ���̴� ������ �ҷ����� DirectX�� GPU���� ��� �����ϵ��� �ϴ� ���� ��
// ���̾ƿ��� �����ϰ� ��� ���� ������ �����Ͱ� GPU���� ���Ǵ��� �� �� ����
// �� ���̾ƿ��� modelclass.h�� color.vs�� ����� VertexType�� ��ġ�� �ʿ䰡 ����
bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;


	// Initialize the pointers this function will use to null.
	// �� �Լ����� ����ϴ� �����͵��� null�� ����
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;


	// ���̴� ���α׷��� ���۷� ������ ��
	// ���̴� ������ �̸�, ���̴��� �̸�, ���̴��� ����(DirectX 11������ 5.0). ���̴��� ������ �� ���۸� ���ڷ� �Ѱ���
	// ���� ������ ������ �����ϸ� ���� �޼����� errorMessage ���ڿ��� ����ϰ�, �̰��� �ٸ� �Լ��� �� �� ����
	// ���� errorMessage ���� �����Ѵٸ� ���̴� ������ ���ٴ� �ǹ��̹Ƿ� �� ������ �˷��ִ� ���̾�α׸� �����ϵ��� ��

    // Compile the vertex shader code.
	// ���� ���̴��� ������ ��
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		// ���̴��� �����Ͽ� �����ϸ� ���� �޼����� ���
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		// ���� �޼����� ���ٸ� ���̴� ������ ã�� ���� ����
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Compile the pixel shader code.
	// �ȼ� ���̴��� ������ ��
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		// ���̴� �������� �����ϸ� ���� �޼����� ���
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		// ���� �޼����� ���ٸ� �ܼ��� ���̴� ������ ã�� ���� ����
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// ���� ���̴��� �ȼ� ���̴��� ���۷� �� �����ϵǸ� �̸� �̿��Ͽ� ���̴� ��ü�� ���� �� ����
	// ���⼭ ���� �����͸� ���� ���̴��� �ȼ� ���̴��� �������̽��μ� ����� ����

    // Create the vertex shader from the buffer.
	// ���۷κ��� ���� ���̴��� ����
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
	// ���۷κ��� �ȼ� ���̴��� ����
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}



	// ���̴����� ����� ���� �������� ���̾ƿ��� ����
	// �� ���̴������� ��ġ ���Ϳ� ���� ���͸� ����ϹǷ� ���̾ƿ��� ������ ������ ũ�⸦ �����ϴ� �� ���̾ƿ��� ����
	
	// SemanticName�� �� ��Ұ� ���̾ƿ����� ��� ���Ǵ��� �˷��ֹǷ� ���̾ƿ����� ���� ���� ä������ �� �׸���
	// �켱 �� �ٸ� ��ҵ� �߿��� POSITION�� ����, TEXTURE�� �� ��°�� ó��
	
	// �� ���� ���̾ƿ����� �߿��� �κ��� Format
	// ��ġ ���Ϳ� ���ؼ��� DXGI_FORMAT_R32G32B32_FLOAT�� ����ϰ� �ؽ��� ���Ϳ� ���ؼ��� DXGI_FORMAT_R32G32_FLOAT�� ���
	
	// ���������� ������ ���� ���ۿ� �����Ͱ� ��� �迭�Ǵ��� �˷��ִ� AlignedBytrOffset
	// �� ���̾ƿ����� ó�� 12byte�� ��ġ ���Ϳ� ����ϰ� ���� 16byte�� �������� ����� ������ �˷���� �ϴµ�, 
	// AlignedByteOffset�� �� ��Ұ� ��� �����ϴ��� ������
	// ���⼭ ���� ���� �Է��ϱ⺸�� D3D11_APPEND_ALIGNED_ELEMENT�� �����Ͽ� �ڵ����� �˾Ƴ����� ��

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	// ���� �Է� ���̾ƿ� description�� �ۼ�
	// �� ������ ModelClass�� ���̴��� �ִ� VertexType�� ��ġ�ؾ� ��
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;


	// ���̾ƿ��� description�� ä������ �̰��� ũ�⸦ ������ D3D ����̽��� ����Ͽ� �Է� ���̾ƿ��� ������
	// ���̾ƿ��� �����Ǹ� ����/�ȼ� ���̴� ���۵��� �� �̻� ������ �����Ƿ� �Ҵ��� ������

	// Get a count of the elements in the layout.
	// ���̾ƿ��� ��� ������ ������
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	// ���� �Է� ���̾ƿ��� ����
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &m_layout);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	// �� �̻� ������ �ʴ� ���� ���̴� ���ۿ� �ȼ� ���̴� ���۸� ����
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;


	// ���̴��� ����ϱ� ���� ������ �ܰ踦 ��� ����(constant buffer)
	// ���� ���̴����� ���Ҵ� ��ó�� ������ �� �ϳ��� ��� ���۸� ������ �ֱ� ������ ���⼭ �װ��� �����Ͽ� ���̴��� ���� �������̽��� ��� �� �� ����
	// �� �����Ӹ��� ��� ���۸� ������Ʈ�ϱ� ������ ������ ����� ������ �� �ʿ䰡 ����
	// BindFlags�� ��� ���۸� �� ���۷� �Ѵٴ� ���� ����
	// CPUAccessFlags�� �뵵�� ���߾�� �ϱ� ������ D3D11_CPU_ACCESS_WRITE�� �����Ǿ�� ��
	// �� description�� ä������ ��� ������ �������̽��� ����� �̿� SetShaderParameters�Լ��� �̿��Ͽ� ���̴��� ���� �����鿡 ������ �� �ֵ��� ��

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	// ���� ���̴��� �ִ� ��� ��� ������ description�� �ۼ�
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	// ��� ���� �����͸� ����� �� Ŭ�������� ���� ���̴� ��� ���ۿ� ������ �� �ְ� ��
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Create a texture sampler state description.
	// �ؽ��� ���÷� ���� description�� �ۼ�
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	// �ؽ��� ���÷� ���¸� ����
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


// InitializeShader �Լ����� �����ߴ� 4���� �������̽����� ������
void TextureShaderClass::ShutdownShader()
{
	// Release the sampler state.
	// ���û��¸� ������
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the matrix constant buffer.
	// ��� ���۸� ������
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	// ���̾ƿ��� ������
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	// �ȼ� ���̴��� ������
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	// ���� ���̴��� ������
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}


// ���� / �ȼ� ���̴��� ������ ���� ������ ���� ������� ���� �޼����� ����ϴ� ���� ��
void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	// ���� �޼����� ��� �ִ� ���ڿ� ������ �����͸� ������
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	// �޼����� ���̸� ������
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	// ������ ���� �ȿ� �޼����� ���
	fout.open("shader-error.txt");

	// Write out the error message.
	// ���� �޼����� ��
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	// ������ ����
	fout.close();

	// Release the error message.
	// ���� �޼����� ��ȯ��
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	// ������ ������ ������ �˾� �޼����� �˷���
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


// ���̴��� ���� ������ ���� �ٷ� �� �ֵ��� �ϱ� ���� �������
// �� �Լ����� ���� ��ĵ��� GraphicsClass���� ������� ����
bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
											 D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;


	// Transpose the matrices to prepare them for the shader.
	// ����� transpose�Ͽ� ���̴����� ����� �� �ְ� ��
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	// ��� ������ ������ �� �� �ֵ��� ���
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	// ��� ������ �����Ϳ� ���� �����͸� ������
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	// ��� ���ۿ� ����� ������
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	// ��� ������ ����� ǰ
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	// ���� ���̴������� ��� ������ ��ġ�� ������
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	// ���� ���̴��� ��� ���۸� �ٲ� ������ �ٲ�
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resource in the pixel shader.
	// �ȼ� ���̴��� ���̴� �ؽ��� ���ҽ��� 0���� ����
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}


// Render �Լ����� �Ҹ��� �� ��° �Լ���
// SetShaderParameters �Լ����� ���� ȣ��Ǿ� ���̴��� ���ڵ��� �ùٷ� �����ϰ� ��
// ���� ���� �ϴ� ���� �Է� ���̾ƿ��� �Է� ������� �����ϴ� ������ GPU ���� ������ �ڷᱸ���� �˰Ե�
// �� ��° �ܰ�� ���� ���۸� �׸��� ���� ���� ���̴��� �ȼ� ���̴��� �����ϴ� ��
// ���̴��� �����Ǹ� D3D ����̽� ���ؽ�Ʈ���� DirectX11�� DrawInderxed �Լ��� ����Ͽ� �ﰢ���� �׷���
// �� �Լ��� ȣ��� ���Ŀ� �𵨸��� �׷���
void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	// ���� �Է� ���̾ƿ��� ������
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
	// �ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� ������
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	// �ȼ� ���̴��� ���÷� ���¸� ������
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	// �ﰢ���� �׸�
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}