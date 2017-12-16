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


	// ���ؽ�, �ȼ� ���̴��� �ʱ�ȭ�մϴ�.
	result = InitializeShader(device, hwnd, L"color.vs", L"color.ps");
	if (!result)
	{
		return false;
	}

	return true;
}
void ColorShaderClass::Shutdown()
{
	// ���ؽ�, �ȼ� ���̴��� ���� ������Ʈ���� �����մϴ�.
	ShutdownShader();

	return;
}
bool ColorShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	bool result;


	// �������ϴµ� ����� ���̴� �Ķ���͵��� �����մϴ�.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	// ���� �غ�� ���۸� ���̴��� �������մϴ�.
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


	// �� �Լ����� ����� �����͸� null�� �ʱ�ȭ �մϴ�.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;
	// ���ؽ� ���̴� �ڵ带 ������ �մϴ�.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);

	if (FAILED(result))
	{
		// ���� ���̴� �������� ������ ��� �����޽����� ������ ������ �ֽ��ϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// �����޽����� ���� ��� �����ϰ� ���� �������� ã�� �� ���ٰ� �޽����ڽ��� ����մϴ�.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}
	// �ʼ� ���̴� �ڵ带 ������ �մϴ�.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);

	if (FAILED(result))
	{
		// ���� ���̴� �������� ������ ��� �����޽����� ������ ������ �ֽ��ϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// �����޽����� ���� ��� �����ϰ� ���� �������� ã�� �� ���ٰ� �޽����ڽ��� ����մϴ�.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}
	// ���۷κ��� ���ؽ� ���̴� ����.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// ���۷κ��� �ȼ� ���̴� ����.Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}
	// ���̴��� ������ �������� ���̾ƿ��� �����մѴ�.
	// �� ������ ���̴��� ModelClass�� �ִ� VertexType ����ü�� ��ġ�� �ʿ��մϴ�.
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
	// ���̾ƿ� ��� ������ ����ϴ�. (���⼭�� 2��)
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// ���ؽ� �Է� ���̾ƿ� ����.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// �� �̻� �ʿ� ���� ���ؽ�, �ȼ� ���̴� ���۸� �����մϴ�.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;
	// ���ؽ� ���̴��� �ִ� ���� ��� ��� ������ description�� �ۼ��մϴ�.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// ��� ���� �����͸� ����ϴ�. ���� �� Ŭ�������� ���ؽ� ���̴� ��� ���ۿ� ������ �� �ֽ��ϴ�.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}
void ColorShaderClass::ShutdownShader()
{
	// ��� ��� ���� ����.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// ���̾ƿ� ����.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// �ȼ� ���̴� ����.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// ���ؽ� ���̴� ����.
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


	// ���� �޽��� �ؽ�Ʈ ������ �����͸� ����ϴ�.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// �޽����� ���̸� ����ϴ�.
	bufferSize = errorMessage->GetBufferSize();

	// �����޽����� ����� ������ ���ϴ�.
	fout.open("shader-error.txt");

	// ���� �޽����� ����մϴ�.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// ������ �ݽ��ϴ�.
	fout.close();

	// �����޽��� ����.
	errorMessage->Release();
	errorMessage = 0;

	// ������ ���� �ؽ�Ʈ ������ ������ Ȯ���ϵ��� �޽��� �˾�â�� ���ϴ�.
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
	// ���̴��� ����� �� �ְ� ��ġ��ķ� �ٲߴϴ�.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// ��� ���۸� ��޴ϴ�. ���� ������ �� �ֽ��ϴ�.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// ������� ���� �����Ϳ� ���� �����͸� ����ϴ�.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// ������� ������ ��ĵ��� �����մϴ�.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// ������ۿ� ���� ����� Ǳ�ϴ�.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// ���ؽ� ���̴��� ��� ���� ��ġ ����.
	bufferNumber = 0;

	// ���������� ������Ʈ�� ���� �Բ� ���ؽ� ���̴��� ��� ���۸� �����մϴ�.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}
void ColorShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// ���ؽ� �Է� ���̾ƿ��� ����.
	deviceContext->IASetInputLayout(m_layout);

	// �ﰢ�� �������� ����� ���ؽ�, �ȼ� ���̴� ����.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// �ﰢ�� ������.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}










