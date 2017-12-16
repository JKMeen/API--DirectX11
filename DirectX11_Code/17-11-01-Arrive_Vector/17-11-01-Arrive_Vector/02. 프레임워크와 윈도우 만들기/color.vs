////////////////////////////////////////////////////////////////////////////////
// Filename: texture.vs
////////////////////////////////////////////////////////////////////////////////

// 셰이더 프로그램은 우선 전역 변수로부터 시작함
// 이 전역 변수들은 C++코드에서 접근하여 수정할 수 있음
// int나 float와 같은 타입 변수들을 사용할 수 있고, 셰이더 프로그램을 사용하기 전에 외부적으로 설정할 수 있음
// 보통 대부분의 전역 변수들은 'cbuffer'라고 불리는 버퍼 객체 타입에 넣게 됨
// 이 버퍼들을 조직하는 일은 셰이더의 효율적인 실행뿐만 아니라
// 그래픽 카드가 어떻게 이 버퍼들을 저장하는지와도 관련있음

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
struct VertexInputType			// 정점 입력 타입
{
    float4 position : POSITION; // x, y, z, w 위치를 가지는 벡터
	float4 color : COLOR;		// r, g, b, a 색상을 가지는 타입
    //float2 tex : TEXCOORD0;
};

struct PixelInputType			// 픽셀 입력 타입
{
    float4 position : SV_POSITION;	// x, y, z, w 위치를 가지는 벡터, GPU에 사용할 변수, 픽셀 셰이더에서 동작
	float4 color : COLOR;			// r, g, b, a 색상을 가지는 타입
    //float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
// 정점 셰이더
// 정점 버퍼의 모든 각 정점에 대해 호출됨
// 정점 셰이더의 입력으로 들어오는 것은 VertexInputType와 같이 셰이더의 소스에서
// 정의한 자료구조와 그 구조가 맞아떨어져야 함
// 정점 셰이더의 처리 결과는 픽셀 셰이더로 보내지고, PixelInputType 형태로 출력
// 입력된 정점들은 들어와서 월드, 뷰, 사영 행렬과 곱셈 연산을 하게 됨
// 3D세계에서의 정점을 2D화면의 위치로 지정해줌
// 출력으로 나온 데이터들이 색상을 할당받은 뒤에는 픽셀 셰이더의 입력 데이터로 사용
// 위치 지정을 위해 XYZ좌표만을 사용하기 때문에 입력 변수 W의 값은 1.0으로 설정하였음

PixelInputType ColorVertexShader(VertexInputType input){
	PixelInputType output;
	
	// 올바르게 행렬 연산을 하기 위해 position 벡터를 w까지 있는 4성분이 있는 것으로 사용
	input.position.w = 1.0f;

	// 정점의 위치를 월드, 뷰, 사영의 순으로 계산
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// 픽셀 셰이더에서 사용하기 위해 입력 색상을 저장
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