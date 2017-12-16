////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"
#include <stdlib.h>
#include <time.h>




/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

	int ModelNum;
	int Max_ModelNum;
	int Select;
	void Arrive(ModelClass *m_Target, ModelClass *m_Model, float max_sp, int dec, float DeT, float max_distance);
	void Flee(ModelClass *m_Target, ModelClass *m_Model, float max_sp, float max_distance);
	void Wander(ModelClass *m_Model, float max_sp, float radius, float r_distance, int max_angle);
	void Around(ModelClass *m_Target, ModelClass *m_Model, float max_sp, int dec, float DeT, float max_distance, int sign);
	void Collision(ModelClass *m_Target, ModelClass *m_Model, float max_sp);

private:
	bool Render();

private:
	ColorShaderClass* m_ColorShader;

public:
	D3DClass* m_D3D;
	

public:
	ModelClass* m_Model[100];
	CameraClass* m_Camera;
};

#endif