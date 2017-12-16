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


// 셰이더의 초기화를 수행하는 함수를 호출하며, texture.vs, texture.ps의 HLSL셰이더 파일의 이름을 넘겨줌
bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	// 정점 셰이더와 픽셀 셰이더를 초기화 함
	result = InitializeShader(device, hwnd, L"../Engine/texture.vs", L"../Engine/texture.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


// 셰이더를 정리하는 함수를 호출
void TextureShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	// 정점 셰이더와 픽셀 셰이더 및 그와 관련된 것들을 반환함
	ShutdownShader();

	return;
}


// Render 함수에서는 우선 SetshaderParameters 함수를 사용하여 셰이더에서 사용될 인자들을 설정함
// 인자들이 설정되면 RenderShader 함수를 호출하여 HLSL 셰이더를 이용하여 모델 그리기를 수행함
bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
								D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	// 렌더링에 사용할 셰이더의 인자를 입력
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	// 셰이더를 이용하여 준비된 버퍼를 그림
	RenderShader(deviceContext, indexCount);

	return true;
}


// 실제로 셰이더 파일을 불러오고 DirectX와 GPU에서 사용 가능하도록 하는 일을 함
// 레이아웃을 세팅하고 어떻게 정점 버퍼의 데이터가 GPU에서 사용되는지 볼 수 있음
// 이 레이아웃은 modelclass.h와 color.vs에 선언된 VertexType와 일치할 필요가 있음
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
	// 이 함수에서 사용하는 포인터들을 null로 설정
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;


	// 셰이더 프로그램을 버퍼로 컴파일 함
	// 셰이더 파일의 이름, 셰이더의 이름, 셰이더의 버전(DirectX 11에서는 5.0). 셰이더가 컴파일 될 버퍼를 인자로 넘겨줌
	// 만약 컴파일 과정이 실패하면 에러 메세지를 errorMessage 문자열에 출력하고, 이것은 다른 함수로 볼 수 있음
	// 만약 errorMessage 없이 실패한다면 셰이더 파일이 없다는 의미이므로 그 내용을 알려주는 다이얼로그를 생성하도록 함

    // Compile the vertex shader code.
	// 정점 셰이더를 컴파일 함
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		// 셰이더가 컴파일에 실패하면 에러 메세지를 기록
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		// 에러 메세지가 없다면 셰이더 파일을 찾지 못한 것임
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Compile the pixel shader code.
	// 픽셀 셰이더를 컴파일 함
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		// 셰이더 컴파일이 실패하면 에러 메세지를 기록
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		// 에러 메세지가 없다면 단순히 셰이더 파일을 찾지 못한 것임
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// 정점 셰이더와 픽셀 셰이더가 버퍼로 잘 컴파일되면 이를 이용하여 셰이더 객체를 만들 수 있음
	// 여기서 나온 포인터를 정점 셰이더와 픽셀 셰이더의 인터페이스로서 사용할 것임

    // Create the vertex shader from the buffer.
	// 버퍼로부터 정점 셰이더를 생성
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
	// 버퍼로부터 픽셀 셰이더를 생성
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}



	// 셰이더에서 사용할 정점 데이터의 레이아웃을 생성
	// 이 셰이더에서는 위치 벡터와 색상 벡터를 사용하므로 레이아웃에 각각의 벡터의 크기를 포함하는 두 레이아웃을 만듦
	
	// SemanticName은 이 요소가 레이아웃에서 어떻게 사용되는지 알려주므로 레이아웃에서 가장 먼저 채워져야 할 항목임
	// 우선 두 다른 요소들 중에서 POSITION을 먼저, TEXTURE를 두 번째로 처리
	
	// 그 다음 레이아웃에서 중요한 부분은 Format
	// 위치 벡터에 대해서는 DXGI_FORMAT_R32G32B32_FLOAT를 사용하고 텍스쳐 벡터에 대해서는 DXGI_FORMAT_R32G32_FLOAT를 사용
	
	// 마지막으로 주의할 것은 버퍼에 데이터가 어떻게 배열되는지 알려주는 AlignedBytrOffset
	// 이 레이아웃에서 처음 12byte를 위치 벡터에 사용하고 다음 16byte를 색상으로 사용할 것임을 알려줘야 하는데, 
	// AlignedByteOffset이 각 요소가 어디서 시작하는지 보여줌
	// 여기서 직접 값을 입력하기보다 D3D11_APPEND_ALIGNED_ELEMENT를 지정하여 자동으로 알아내도록 함

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	// 정점 입력 레이아웃 description을 작성
	// 이 설정은 ModelClass와 셰이더에 있는 VertexType와 일치해야 함
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


	// 레이아웃의 description이 채워지면 이것의 크기를 가지고 D3D 디바이스를 사용하여 입력 레이아웃을 생성함
	// 레이아웃이 생성되면 정점/픽셀 셰이더 버퍼들은 더 이상 사용되지 않으므로 할당을 해제함

	// Get a count of the elements in the layout.
	// 레이아웃의 요소 개수를 가져옴
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	// 정점 입력 레이아웃을 생성
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &m_layout);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	// 더 이상 사용되지 않는 정점 셰이더 버퍼와 픽셀 셰이더 버퍼를 해제
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;


	// 셰이더를 사용하기 위한 마지막 단계를 상수 버퍼(constant buffer)
	// 정점 셰이더에서 보았던 것처럼 지금은 단 하나의 상수 버퍼를 가지고 있기 때문에 여기서 그것을 세팅하여 셰이더에 대한 인터페이스를 사용 할 수 있음
	// 매 프레임마다 상수 버퍼를 업데이트하기 때문에 버퍼의 사용은 동적이 될 필요가 있음
	// BindFlags로 상수 버퍼를 이 버퍼로 한다는 것을 설정
	// CPUAccessFlags도 용도에 맞추어야 하기 때문에 D3D11_CPU_ACCESS_WRITE로 설정되어야 함
	// 이 description이 채워지면 상수 버퍼의 인터페이스를 만들고 이와 SetShaderParameters함수를 이용하여 셰이더의 내부 변수들에 접근할 수 있도록 함

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	// 정점 셰이더에 있는 행렬 상수 버퍼의 description을 작성
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 함
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Create a texture sampler state description.
	// 텍스쳐 샘플러 상태 description을 작성
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
	// 텍스쳐 샘플러 상태를 생성
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


// InitializeShader 함수에서 생성했던 4개의 인터페이스들을 정리함
void TextureShaderClass::ShutdownShader()
{
	// Release the sampler state.
	// 샘플상태를 해제함
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the matrix constant buffer.
	// 상수 버퍼를 해제함
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	// 레이아웃을 해제함
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	// 픽셀 셰이더를 해제함
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	// 정점 셰이더를 해제함
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}


// 정점 / 픽셀 셰이더의 컴파일 도중 오류로 인해 만들어진 에러 메세지를 출력하는 일을 함
void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	// 에러 메세지를 담고 있는 문자열 버퍼의 포인터를 가져옴
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	// 메세지의 길이를 가져옴
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	// 파일을 열고 안에 메세지를 기록
	fout.open("shader-error.txt");

	// Write out the error message.
	// 에러 메세지를 씀
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	// 파일을 닫음
	fout.close();

	// Release the error message.
	// 에러 메세지를 반환함
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	// 컴파일 에러가 있음을 팝업 메세지로 알려줌
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


// 셰이더의 전역 변수를 쉽게 다룰 수 있도록 하기 위해 만들어짐
// 이 함수에서 사용된 행렬들은 GraphicsClass에서 만들어진 것임
bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
											 D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;


	// Transpose the matrices to prepare them for the shader.
	// 행렬을 transpose하여 셰이더에서 사용할 수 있게 함
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	// 상수 버퍼의 내용을 쓸 수 있도록 잠금
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	// 상수 버퍼의 데이터에 대한 포인터를 가져옴
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	// 상수 버퍼에 행렬을 복사함
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	// 상수 버퍼의 잠금을 품
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	// 정점 셰이더에서의 상수 버퍼의 위치를 설정함
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	// 정점 셰이더의 상수 버퍼를 바뀐 값으로 바꿈
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resource in the pixel shader.
	// 픽셀 셰이더의 셰이더 텍스쳐 리소스를 0으로 설정
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}


// Render 함수에서 불리는 두 번째 함수임
// SetShaderParameters 함수보다 먼저 호출되어 셰이더의 인자들을 올바로 세팅하게 됨
// 가장 먼저 하는 것은 입력 레이아웃을 입력 어셈블러에 연결하는 함으로 GPU 정점 버퍼의 자료구조를 알게됨
// 두 번째 단계는 정점 버퍼를 그리기 위한 정점 셰이더와 픽셀 셰이더를 설정하는 것
// 셰이더가 설정되면 D3D 디바이스 컨텍스트에서 DirectX11의 DrawInderxed 함수를 사용하여 삼각형을 그려냄
// 이 함수가 호출된 이후에 모델링이 그려짐
void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	// 정점 입력 레이아웃을 설정함
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정함
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	// 픽셀 셰이더의 샘플러 상태를 세팅함
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	// 삼각형을 그림
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}