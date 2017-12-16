////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"
#define ModelNum 2

GraphicsClass::GraphicsClass()
{
	Select = 1;

	m_D3D = 0;
	m_Camera = 0; 
	for (int i = 0; i < 10; i++)
		m_Model[i] = 0; 
	m_ColorShader = 0;

	// Create the model object. 
	// �簢��
	for (int i = 0; i < ModelNum; i++){
		m_Model[i] = new ModelClass;
		/*if (!m_Model[i]) {
			return false;
		}*/
	}
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;


	// Create the Direct3D object.
	// Direct3D ��ü�� ����
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	// Direct3D ��ü�� �ʱ�ȭ
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object. 
	m_Camera = new CameraClass; 
	if(!m_Camera) { 
		return false; 
	} 
	
	// Set the initial position of the camera. 
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f); 
	

	// Initialize the model object.
	// float x, float y, int m_vCount, int m_iCount, float radius, int nPoints, float angle, Device

	// �簢��
	result = m_Model[0]->Initialize(12, 12, 1.5, 4, m_D3D->GetDevice());
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		false;
	}
	m_Model[0]->Set_Position(vec3d(0, 0, 1));
	m_Model[0]->Set_Velocity(vec3d(0, 0, 0));
	m_Model[0]->Set_Color(vec3d(0, 0, 250));

	srand((unsigned)time(NULL));
	for (int i = 1; i < ModelNum; i++){
		// �Ȱ���
		result = m_Model[i]->Initialize(24, 24, 2.0, 8, m_D3D->GetDevice());
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			false;
		}
		m_Model[i]->Set_Position(vec3d(rand() % 99 - 49, rand() % 99 - 49, 1));
		m_Model[i]->Set_Velocity(vec3d(0, 0, 0));
		m_Model[i]->Set_Color(vec3d(rand()%255, 0, 0));
	}


	// Create the color shader object. 
	m_ColorShader = new ColorShaderClass; 
	if(!m_ColorShader) { 
		return false; 
	} 
	
	// Initialize the color shader object. 
	result = m_ColorShader->Initialize(m_D3D->GetDevice(), hwnd); 
	if(!result) { 
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK); 
		return false; 
	}

	return true;
}



void GraphicsClass::Shutdown()
{
	// Release the D3D object.
	// D3D ��ü�� ��ȯ
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	// Release the model object. 
	for (int i = 0; i < ModelNum; i++){
		if (m_Model[i]) {
			m_Model[i]->Shutdown();
			delete m_Model[i];
			m_Model[i] = 0;
		}
	}
	
	
	// Release the camera object. 
	if(m_Camera) { 
		delete m_Camera; 
		m_Camera = 0; 
	} 
	
	// Release the D3D object. 
	if(m_D3D) { 
		m_D3D->Shutdown(); 
		delete m_D3D; 
		m_D3D = 0; 
	}

	return;
}


void GraphicsClass::Arrive(){
	// �Ȱ����� �i�ƿ��� �ڵ�
	for (int i = 1; i < ModelNum; i++){
		float x = m_Model[0]->Model->location->v[0] - m_Model[i]->Model->location->v[0];
		float y = m_Model[0]->Model->location->v[1] - m_Model[i]->Model->location->v[1];
		float z = m_Model[0]->Model->location->v[2] - m_Model[i]->Model->location->v[2];
		float distance = sqrt(x*x + y*y + z*z); // A�� B�� �Ÿ�
		float max_speed = rand() % 6 + 3;//5.0;
		if (distance > 0){
			// deceleration(����)
			int deceleration = rand() % 250 + 50;//50;
			float DecelerationTweaker = 0.3;

			// ���ϴ� ������ �־��� ��� ��ǥ�� �����ϱ� ���� �䱸�Ǵ� �ӵ��� ���
			float speed = distance / ((float)deceleration * DecelerationTweaker);

			// �ִ� ���ǵ� ����
			if (speed >= max_speed)
				speed = max_speed;

			// ����ȭ
			vec3d DesiredVelocity = vec3d(x, y, z) * speed / distance;
			m_Model[i]->Set_Velocity(DesiredVelocity);
		}
		else {
			m_Model[i]->Set_Velocity(vec3d(0, 0, 0));
		}

		vec3d v = vec3d(m_Model[i]->Model->velocity->v[0], m_Model[i]->Model->velocity->v[1], m_Model[i]->Model->velocity->v[2]);
		m_Model[i]->Set_Position(v);
	}
}

void GraphicsClass::Flee(){
	// �Ȱ����� �i�ƿ��� �ڵ�
	for (int i = 1; i < ModelNum; i++){
		float x = m_Model[0]->Model->location->v[0] - m_Model[i]->Model->location->v[0];
		float y = m_Model[0]->Model->location->v[1] - m_Model[i]->Model->location->v[1];
		float z = m_Model[0]->Model->location->v[2] - m_Model[i]->Model->location->v[2];
		float distance = sqrt(x*x + y*y + z*z); // A�� B�� �Ÿ�
		float max_speed = rand() % 6 + 3;//5.0;
		if (distance > 0){
			// deceleration(����)
			int deceleration = rand() % 250 + 50;//50;
			float DecelerationTweaker = 0.3;

			// ���ϴ� ������ �־��� ��� ��ǥ�� �����ϱ� ���� �䱸�Ǵ� �ӵ��� ���
			float speed = distance / ((float)deceleration * DecelerationTweaker);

			// �ִ� ���ǵ� ����
			if (speed >= max_speed)
				speed = max_speed;

			// ����ȭ
			vec3d DesiredVelocity = vec3d(x, y, z) * speed / distance;
			m_Model[i]->Set_Velocity(DesiredVelocity);
		}
		else {
			m_Model[i]->Set_Velocity(vec3d(0, 0, 0));
		}

		vec3d v = vec3d(m_Model[i]->Model->velocity->v[0], m_Model[i]->Model->velocity->v[1], m_Model[i]->Model->velocity->v[2]);
		m_Model[i]->Set_Position(v);
	}
}

// ��Ʈ��
bool GraphicsClass::Frame()
{
	bool result;

	switch (Select){

	// Arrive
	case 1: Arrive(); break;
	case 2: Flee(); break;

	}
	
	// Model->x, Model->y, Model->m_vCount, Model->m_iCount, Model->radius, Model->nPoints, Model->r, Model->g, Model->b, device
	for (int i = 0; i < ModelNum; i++){
		result = m_Model[i]->Initialize(m_Model[i]->Model->m_vCount, m_Model[i]->Model->m_iCount,
										m_Model[i]->Model->radius, m_Model[i]->Model->nPoints,
										m_D3D->GetDevice());
		if (!result)
		{
			return false;
		}
	}

	// Render the graphics scene.
	// �׷��� �������� ����
	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render()
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix; 
	bool result;

	// Clear the buffers to begin the scene.
	// �� �׸��⸦ �����ϱ� ���� ������ ������ ����
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects. 
	m_Camera->GetViewMatrix(viewMatrix); 
	m_D3D->GetWorldMatrix(worldMatrix); 
	m_D3D->GetProjectionMatrix(projectionMatrix); 
		
	for (int i = 0; i < ModelNum; i++){
		// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing. 
		m_Model[i]->Render(m_D3D->GetDeviceContext());

		// Render the model using the color shader. 
		result = m_ColorShader->Render(m_D3D->GetDeviceContext(), m_Model[i]->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		if (!result) {
			return false;
		}
	}

	// Present the rendered scene to the screen.
	// ���ۿ� �׷��� ���� ȭ�鿡 ǥ��
	m_D3D->EndScene();

	return true;
}