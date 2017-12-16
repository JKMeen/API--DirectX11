////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"

GraphicsClass::GraphicsClass()
{
	Max_ModelNum = 99;
	ModelNum = 4;
	Select = 1;

	m_D3D = 0;
	m_Camera = 0; 
	for (int i = 0; i < 10; i++)
		m_Model[i] = 0; 
	m_ColorShader = 0;

	// Create the model object. 
	// 사각형
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
	// Direct3D 객체를 생성
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	// Direct3D 객체를 초기화
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
	// int m_vCount, int m_iCount, float radius, int nPoints, float angle, Device

	// 사각형
	result = m_Model[0]->Initialize(3, 1.5, m_D3D->GetDevice());
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		false;
	}
	m_Model[0]->Model->name = "Player";
	m_Model[0]->Set_Position(vec3d(0, 0, 1));
	m_Model[0]->Set_Velocity(vec3d(0, 0, 0));
	m_Model[0]->Set_Color(vec3d(0, 0, 250));

	// 위성
	result = m_Model[1]->Initialize(6, 2.0, m_D3D->GetDevice());
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		false;
	}
	m_Model[1]->Model->name = "Settle";
	m_Model[1]->Set_Position(vec3d(0, 0, 1));
	m_Model[1]->Set_Velocity(vec3d(0, 0, 0));
	m_Model[1]->Set_Color(vec3d(0, 250, 0));

	// 적
	srand((unsigned)time(NULL));
	for (int i = 2; i < ModelNum; i++){
		// 팔각형
		result = m_Model[i]->Initialize(8, 2.5, m_D3D->GetDevice());
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			false;
		}
		m_Model[i]->Model->name = "Enemy";
		m_Model[i]->Set_Position(vec3d(rand() % 99 - 49, rand() % 99 - 49, 1));
		m_Model[i]->Set_Velocity(vec3d(0, 0, 0));
		m_Model[i]->Set_Color(vec3d(rand() % 255, 0, rand() % 255));
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
	// D3D 객체를 반환
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


// 도착하기
void GraphicsClass::Arrive(ModelClass *m_Target, ModelClass *m_Model, float max_sp, int dec, float DeT, float max_distance){
	// 팔각형이 쫒아오는 코드
	float x = m_Target->Model->location->v[0] - m_Model->Model->location->v[0];
	float y = m_Target->Model->location->v[1] - m_Model->Model->location->v[1];
	float z = m_Target->Model->location->v[2] - m_Model->Model->location->v[2];
	float distance = sqrt(x*x + y*y + z*z); // A와 B의 거리

	float max_speed = max_sp;
	
	int deceleration = dec; // deceleration(감속)
	float DecelerationTweaker = DeT;

	if (distance > 0){
		// 원하는 감속이 주어진 경우 목표에 도달하기 위해 요구되는 속도를 계산
		m_Model->Model->speed = distance / ((float)deceleration * DecelerationTweaker);

		// 최대 스피드 제한
		if (m_Model->Model->speed >= max_speed)
			m_Model->Model->speed = max_speed;

		// 정규화
		vec3d DesiredVelocity = vec3d(x, y, z) * m_Model->Model->speed / distance;
		m_Model->Set_Velocity(DesiredVelocity);
	}
	else {
		m_Model->Set_Velocity(vec3d(0, 0, 0));
	}

	//vec3d v = vec3d(m_Model->Model->velocity->v[0], m_Model->Model->velocity->v[1], m_Model->Model->velocity->v[2]);
	//m_Model->Set_Position(v);
}


// 도망가기
void GraphicsClass::Flee(ModelClass *m_Target, ModelClass *m_Model, float max_sp, float max_distance){
	float x = m_Target->Model->location->v[0] - m_Model->Model->location->v[0];
	float y = m_Target->Model->location->v[1] - m_Model->Model->location->v[1];
	float z = m_Target->Model->location->v[2] - m_Model->Model->location->v[2];
	float distance = sqrt(x*x + y*y + z*z); // A와 B의 거리
	float max_speed = max_sp;

	// 최대 스피드 제한
	if (m_Model->Model->speed >= max_speed)
		m_Model->Model->speed = max_speed;

	if (distance <= max_distance){
		// 정규화
		float x2 = m_Model->Model->location->v[0] - m_Target->Model->location->v[0];
		float y2 = m_Model->Model->location->v[1] - m_Target->Model->location->v[1];
		float z2 = m_Model->Model->location->v[2] - m_Target->Model->location->v[2];
		m_Model->Model->speed = distance / ((float)0.3 * 50);

		float length = sqrt(x2*x2 + y2*y2 + z2*z2);

		vec3d DesiredVelocity = vec3d(x2 / length, y2 / length, z2 / length) * m_Model->Model->speed;
		m_Model->Set_Velocity(DesiredVelocity);

		//vec3d v = vec3d(m_Model->Model->velocity->v[0], m_Model->Model->velocity->v[1], m_Model->Model->velocity->v[2]);
		//m_Model->Set_Position(v);
	}
	else if (distance > 30){
		//Arrive(this->m_Model[0], this->m_Model[2], 3.0f, 50, 0.3f);
		//m_Model->Set_Velocity(vec3d(0, 0, 0));
		Wander(m_Model, max_sp, 20, 5.0f, 30);
	}
}


// 공전하기
void GraphicsClass::Around(ModelClass *m_Target, ModelClass *m_Model, float max_sp, int dec, float DeT, float max_distance, int sign){
	// 팔각형이 쫒아오는 코드
	float x = m_Target->Model->location->v[0] - m_Model->Model->location->v[0];
	float y = m_Target->Model->location->v[1] - m_Model->Model->location->v[1];
	float z = m_Target->Model->location->v[2] - m_Model->Model->location->v[2];
	float distance = sqrt(x*x + y*y + z*z); // A와 B의 거리

	float max_speed = max_sp;

	int deceleration = dec; // deceleration(감속)
	float DecelerationTweaker = DeT;

	static float sh_angle = 0.0;
	float angle_add = 0.05;// *sign;
	float x1 = (m_Target->Model->location->v[0] + max_distance * cos(sh_angle)) - m_Model->Model->location->v[0];
	float y1 = (m_Target->Model->location->v[1] + max_distance * sin(sh_angle)) - m_Model->Model->location->v[1];
	float z1 = 0.0f;
	float distance2 = sqrt(x1*x1 + y1*y1 + z1*z1);

	m_Model->Model->speed = distance2 / ((float)deceleration * DecelerationTweaker);

	// 최대 스피드 제한
	if (m_Model->Model->speed >= max_speed)
		m_Model->Model->speed = max_speed;

	// 정규화
	vec3d v = vec3d(-m_Model->Model->velocity->v[0], -m_Model->Model->velocity->v[1], -m_Model->Model->velocity->v[2]);
	m_Model->Set_Velocity(v);

	vec3d DesiredVelocity = vec3d(x1, y1, z1) * m_Model->Model->speed / distance2;
	m_Model->Set_Velocity(DesiredVelocity);

	sh_angle += angle_add;


	/*vec3d v = vec3d(m_Model->Model->velocity->v[0], m_Model->Model->velocity->v[1], m_Model->Model->velocity->v[2]);
	m_Model->Set_Position(v);*/
}


int Rand_int(int min, int max){
	return rand() % (max-min) + min;
}


// 배회하기
void GraphicsClass::Wander(ModelClass *m_Model, float max_sp, float radius, float r_distance, int max_angle){
	static int n = 0;

	if (n % 100 == 0){
		int sign = Rand_int(-1, 1);
		float target_angle = rand() % max_angle;
		vec3d target = vec3d(m_Model->Model->location->v[0] + radius*cos(target_angle*sign), m_Model->Model->location->v[1] + radius*sin(target_angle*sign), 0.0f);

		float x = target.v[0] - m_Model->Model->location->v[0];
		float y = target.v[1] - m_Model->Model->location->v[1];
		float z = target.v[2] - m_Model->Model->location->v[2];
		float distance = sqrt(x*x + y*y + z*z); // A와 B의 거리

		m_Model->Model->speed = distance / ((float)0.3 * 50);

		// 최대 스피드 제한
		if (m_Model->Model->speed >= max_sp)
			m_Model->Model->speed = max_sp;

		float length = sqrt(x*x + y*y + z*z);

		vec3d DesiredVelocity = vec3d(x / length, y / length, z / length) * m_Model->Model->speed;
		m_Model->Set_Velocity(DesiredVelocity);

		//vec3d v = vec3d(m_Model->Model->velocity->v[0] + DesiredVelocity.v[0], m_Model->Model->velocity->v[1] + DesiredVelocity.v[1], m_Model->Model->velocity->v[2] + DesiredVelocity.v[2]);
	}

	n++;
}


// 충돌
void GraphicsClass::Collision(ModelClass *m_Target, ModelClass *m_Model, float max_sp){
	float x = m_Target->Model->location->v[0] - m_Model->Model->location->v[0];
	float y = m_Target->Model->location->v[1] - m_Model->Model->location->v[1];
	float z = m_Target->Model->location->v[2] - m_Model->Model->location->v[2];
	float distance = sqrt(x*x + y*y + z*z); // A와 B의 거리
	//float max_speed = max_sp;

	m_Model->Model->speed = distance / ((float)0.3 * 50);

	// 최대 스피드 제한
	if (m_Model->Model->speed >= max_sp)
		m_Model->Model->speed = max_sp;

	if (distance <= m_Model->Model->radius + m_Target->Model->radius){
		// 정규화
		float x2 = m_Model->Model->location->v[0] - m_Target->Model->location->v[0];
		float y2 = m_Model->Model->location->v[1] - m_Target->Model->location->v[1];
		float z2 = m_Model->Model->location->v[2] - m_Target->Model->location->v[2];

		float length = sqrt(x2*x2 + y2*y2 + z2*z2);

		vec3d DesiredVelocity = vec3d(x2 / length, y2 / length, z2 / length) * m_Model->Model->speed;
		m_Model->Set_Velocity(DesiredVelocity);
	}
}


// :::::루트문::::::
bool GraphicsClass::Frame()
{
	bool result;

	static int dt = 0;
	dt += 1;

	if (dt % 10 == 0){

		switch (Select){

			// Arrive :: ModelClass *m_Target, ModelClass *m_Model, float max_sp, int dec, float DeT
			// Flee :: ModelClass *m_Target, ModelClass *m_Model, float max_sp
		case 1: Around(m_Model[0], m_Model[1], 0.1f, 50, 0.3f, 10, 1);
			if (ModelNum > 2){
				for (int i = 2; i < ModelNum; i++){
					Arrive(m_Model[0], m_Model[i], 0.1f, 3, 50, 50);
					Flee(m_Model[1], m_Model[i], 0.1f, 20);
					//Wander(m_Model[i], 0.1f, 5, 10, 90);
				}
			}

			for (int i = 1; i < ModelNum - 1; i++){
				for (int j = i + 1; j < ModelNum; j++){
					Collision(m_Model[i], m_Model[j], 0.1f);
					Collision(m_Model[j], m_Model[i], 0.1f);
				}
			}
			break;

		case 2: Arrive(m_Model[0], m_Model[1], 0.1f, 50, 0.3f, 10); break;

		}
	}
	
	// Model->x, Model->y, Model->m_vCount, Model->m_iCount, Model->radius, Model->nPoints, Model->r, Model->g, Model->b, device
	for (int i = 0; i < ModelNum; i++){
		vec3d v = vec3d(m_Model[i]->Model->velocity->v[0], m_Model[i]->Model->velocity->v[1], m_Model[i]->Model->velocity->v[2]);
		m_Model[i]->Set_Position(v);

		result = m_Model[i]->Initialize(m_Model[i]->Model->nPoints, m_Model[i]->Model->radius, m_D3D->GetDevice());
		if (!result)
		{
			return false;
		}
	}

	// Render the graphics scene.
	// 그래픽 렌더링을 수행
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
	// 씬 그리기를 시작하기 위해 버퍼의 내용을 지움
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
	// 버퍼에 그려진 씬을 화면에 표시
	m_D3D->EndScene();

	return true;
}