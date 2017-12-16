////////////////////////////////////////////////////////////////////////////////
// Filename: colorshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ColorShaderClass.h"

ColorShaderClass::ColorShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
}


ColorShaderClass::ColorShaderClass(const ColorShaderClass& other)
{
}


ColorShaderClass::~ColorShaderClass()
{
}

bool ColorShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// 버텍스, 픽셀 쉐이더를 초기화합니다.
	result = InitializeShader(device, hwnd, L"color.vs", L"color.ps");
	if (!result)
	{
		return false;
	}

	return true;
}
void ColorShaderClass::Shutdown()
{
	// 버텍스, 픽셀 쉐이더와 관련 오브젝트들을 정리합니다.
	ShutdownShader();

	return;
}
bool ColorShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	bool result;


	// 렌더링하는데 사용할 쉐이더 파라미터들을 설정합니다.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	// 이제 준비된 버퍼를 쉐이더로 렌더링합니다.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;


	// 이 함수에서 사용할 포인터를 null로 초기화 합니다.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;
	// 버텍스 쉐이더 코드를 컴파일 합니다.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);

	if (FAILED(result))
	{
		// 만약 쉐이더 컴파일을 실패할 경우 에러메시지에 뭔가가 쓰여져 있습니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// 에러메시지가 없을 경우 간단하게 직접 쉐이파일 찾을 수 없다고 메시지박스로 출력합니다.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}
	// 필셀 쉐이더 코드를 컴파일 합니다.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);

	if (FAILED(result))
	{
		// 만약 쉐이더 컴파일을 실패할 경우 에러메시지에 뭔가가 쓰여져 있습니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// 에러메시지가 없을 경우 간단하게 직접 쉐이파일 찾을 수 없다고 메시지박스로 출력합니다.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}
	// 버퍼로부터 버텍스 쉐이더 생성.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// 버퍼로부터 픽셀 쉐이더 생성.Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}
	// 쉐이더로 전달할 데이터의 레이아웃을 설정합닌다.
	// 이 설정은 쉐이더와 ModelClass에 있는 VertexType 구조체의 일치가 필요합니다.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;
	// 레이아웃 요소 갯수를 얻습니다. (여기서는 2개)
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 버텍스 입력 레이아웃 생성.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// 더 이상 필요 없는 버텍스, 픽셀 쉐이더 버퍼를 해제합니다.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;
	// 버텍스 쉐이더에 있는 동적 행렬 상수 버퍼의 description을 작성합니다.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 상수 버퍼 포인터를 만듭니다. 이제 이 클래스에서 버텍스 쉐이더 상수 버퍼에 접근할 수 있습니다.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}
void ColorShaderClass::ShutdownShader()
{
	// 행렬 상수 버퍼 해제.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// 레이아웃 해제.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// 픽셀 쉐이더 해제.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// 버텍스 쉐이더 해제.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}
void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// 에러 메시지 텍스트 버퍼의 포인터를 얻습니다.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// 메시지의 길이를 얻습니다.
	bufferSize = errorMessage->GetBufferSize();

	// 에러메시지를 기록할 파일을 엽니다.
	fout.open("shader-error.txt");

	// 에러 메시지를 기록합니다.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// 파일을 닫습니다.
	fout.close();

	// 에러메시지 해제.
	errorMessage->Release();
	errorMessage = 0;

	// 컴파일 에러 텍스트 파일을 유저가 확인하도록 메시지 팝업창을 띄웁니다.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}
bool ColorShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;
	// 쉐이더가 사용할 수 있게 전치행렬로 바꿉니다.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// 상수 버퍼를 잠급니다. 이제 수정될 수 있습니다.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// 상수버퍼 내부 데이터에 대한 포인터를 얻습니다.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 상수버퍼 안으로 행렬들을 복사합니다.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// 상수버퍼에 대한 잠금을 풉니다.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// 버텍스 쉐이더의 상수 버퍼 위치 설정.
	bufferNumber = 0;

	// 마지막으로 업데이트된 값과 함께 버텍스 쉐이더의 상수 버퍼를 설정합니다.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}
void ColorShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// 버텍스 입력 레이아웃을 설정.
	deviceContext->IASetInputLayout(m_layout);

	// 삼각형 렌더링에 사용할 버텍스, 픽셀 쉐이더 설정.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 삼각형 렌더링.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}










