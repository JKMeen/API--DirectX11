////////////////////////////////////////////////////////////////////////////////
// Filename: texture.vs
////////////////////////////////////////////////////////////////////////////////

// ���̴� ���α׷��� �켱 ���� �����κ��� ������
// �� ���� �������� C++�ڵ忡�� �����Ͽ� ������ �� ����
// int�� float�� ���� Ÿ�� �������� ����� �� �ְ�, ���̴� ���α׷��� ����ϱ� ���� �ܺ������� ������ �� ����
// ���� ��κ��� ���� �������� 'cbuffer'��� �Ҹ��� ���� ��ü Ÿ�Կ� �ְ� ��
// �� ���۵��� �����ϴ� ���� ���̴��� ȿ������ ����Ӹ� �ƴ϶�
// �׷��� ī�尡 ��� �� ���۵��� �����ϴ����͵� ��������

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType			// ���� �Է� Ÿ��
{
    float4 position : POSITION; // x, y, z, w ��ġ�� ������ ����
	float4 color : COLOR;		// r, g, b, a ������ ������ Ÿ��
    //float2 tex : TEXCOORD0;
};

struct PixelInputType			// �ȼ� �Է� Ÿ��
{
    float4 position : SV_POSITION;	// x, y, z, w ��ġ�� ������ ����, GPU�� ����� ����, �ȼ� ���̴����� ����
	float4 color : COLOR;			// r, g, b, a ������ ������ Ÿ��
    //float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
// ���� ���̴�
// ���� ������ ��� �� ������ ���� ȣ���
// ���� ���̴��� �Է����� ������ ���� VertexInputType�� ���� ���̴��� �ҽ�����
// ������ �ڷᱸ���� �� ������ �¾ƶ������� ��
// ���� ���̴��� ó�� ����� �ȼ� ���̴��� ��������, PixelInputType ���·� ���
// �Էµ� �������� ���ͼ� ����, ��, �翵 ��İ� ���� ������ �ϰ� ��
// 3D���迡���� ������ 2Dȭ���� ��ġ�� ��������
// ������� ���� �����͵��� ������ �Ҵ���� �ڿ��� �ȼ� ���̴��� �Է� �����ͷ� ���
// ��ġ ������ ���� XYZ��ǥ���� ����ϱ� ������ �Է� ���� W�� ���� 1.0���� �����Ͽ���

PixelInputType ColorVertexShader(VertexInputType input){
	PixelInputType output;
	
	// �ùٸ��� ��� ������ �ϱ� ���� position ���͸� w���� �ִ� 4������ �ִ� ������ ���
	input.position.w = 1.0f;

	// ������ ��ġ�� ����, ��, �翵�� ������ ���
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// �ȼ� ���̴����� ����ϱ� ���� �Է� ������ ����
	output.color = input.color;

	return output;
}

/*PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
    return output;
}*/