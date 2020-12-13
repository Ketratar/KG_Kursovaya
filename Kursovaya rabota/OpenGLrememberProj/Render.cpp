#include "Render.h"




#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"
#include <iostream>
#include <sstream>
#include "Texture.h"



bool textureMode = true;
bool lightMode = true;


//��������� ������ ��� ��������� ����
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //��������� ��� ������ ��������
Shader frac;
Shader cassini;


//��������� ������
Vector3 vec0(0, 0, 0);

bool reverse = false; // ���� ��� ��������� ��������, �������� ������ ��� �������� �� ���������� � �� ������ �����
bool stop = false; // ��� ��������� ������

double delta = 0.01; // ��� � ��� ��� �������
double t_max = 0; // ����� ���� ��� �������� ���������� �� �������
double left_right = 0;
double W_angle = 0;
double r = 10; // ��� �������� ��������� �� ����������


// ������������ ���������� ���������� ��� �������� �������� � ������������
float tX = 0, tY = 0, tZ = -8;
float rotX = 0, rotY = 0, rotZ = 0;
float angleBackFrac = 0.4;

//�������� 0.05 - ������; 0.1 - �������; 0.12 - �������
double t_max_flight = 0.05; // �������� ������ ��������
int flight_control = 1; // ������������ �������� ������ ��������

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	Vector3 cameraFront;

	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(vec0.X(), vec0.Y(), vec0.Z());

		//lookPoint.setCoords(0, 0, 0);


		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}

	void SetLookPoint(double x, double y, double z)
	{
		lookPoint.setCoords(x, y, z);
	}

	void SetLookPoint(Vector3 vec)
	{
		lookPoint = vec;
	}

} camera; //������� ������ ������






//����� ���������!
class WASDcamera :public CustomCamera
{
public:
		
	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*(-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}

		LookAt();
	}

} WASDcam;


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����



//������ ���������� ����
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//���������� �������� ����
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

//���������� �������� ������  ����
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}

//����� ���
bool debug = false;

//���������� ������� ������ ����������
void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}	   

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	//if (key == 'S')
	//{
	//	frac.LoadShaderFromFile();
	//	frac.Compile();
	//
	//	s[0].LoadShaderFromFile();
	//	s[0].Compile();
	//
	//	cassini.LoadShaderFromFile();
	//	cassini.Compile();
	//}

	if (key == 'Q')
		Time = 0;

	if (key == 'Z')
	{
		if (!reverse)
		{
			reverse = !reverse;
		}
		else
		{
			reverse = !reverse;
		}
	}

	if (OpenGL::isKeyPressed('M'))
	{
		if (!stop)
			stop = !stop;
		else
		{
			delta = 0.01;
			stop = !stop;
		}

	}

	if (OpenGL::isKeyPressed('C'))
	{
		switch (flight_control)
		{
		case 1:
			flight_control = 2;
			break;
		case 2:
			flight_control = 3;
			break;
		case 3:
			flight_control = 1;
			break;
		}
	}

	//������ ����� ���
	if (OpenGL::isKeyPressed('P'))
	{
		if (debug)
			debug = !debug;
		else
			debug = !debug;
	}

}

void keyUpEvent(OpenGL *ogl, int key)
{

}


double rcamera = 8;



int flag_controlinvert = 1;
float rotFrac = 2.5;

void MyKeyEvent()
{

	float frac = 0.3;
	


	if (OpenGL::isKeyPressed('W'))
	{
		tX -= frac;
		//rotX += rotFrac * 3;
		rotY -= rotFrac / 2;
		if (rotY < -360)
			rotY = 0;
	}
	if (OpenGL::isKeyPressed('A'))
	{
		tY += frac;
		rotZ -= rotFrac;
	}
	if (OpenGL::isKeyPressed('S'))
	{
		tX += frac;
		//rotX -= rotFrac * 3;
		rotY += rotFrac / 2;
		if (rotY > 360)
			rotY = 0;
	}
	if (OpenGL::isKeyPressed('D'))
	{
		tY -= frac;
		rotZ += rotFrac;
	}
	if (OpenGL::isKeyPressed(VK_LEFT))
	{
		left_right += rotFrac / 1.5;
	}
	if (OpenGL::isKeyPressed(VK_RIGHT))
	{
		left_right -= rotFrac / 1.5;
	}




	//��� �������� ������ � ����������� �� ���� ������� �������� ������������ ��� Oy
	if ((rotY <= -90 || rotY >= 90) && !(rotY <= -270 || rotY >= 270))
	{
		if (!OpenGL::isKeyPressed('W') && !OpenGL::isKeyPressed('A') && !OpenGL::isKeyPressed('S') && !OpenGL::isKeyPressed('D') && flag_controlinvert == 1)
		{
			rotFrac *= -1;
			flag_controlinvert = -1;
		}

		Vector3 tmp
		{
			vec0.X() - 3,
			vec0.Y(),
			vec0.Z() + 3
		};
		camera.pos = tmp;
	}
	else
	{
		if (!OpenGL::isKeyPressed('W') && !OpenGL::isKeyPressed('A') && !OpenGL::isKeyPressed('S') && !OpenGL::isKeyPressed('D') && flag_controlinvert == -1)
		{
			rotFrac *= -1;
			flag_controlinvert = 1;
		}



		Vector3 tmp
		{
			vec0.X() + 3,
			vec0.Y(),
			vec0.Z() + 3
		};
		camera.pos = tmp;
	}


	camera.camDist = 30;
	camera.SetLookPoint(vec0);



}


void NonKeyEvent()
{
	//if (!OpenGL::isKeyPressed('W') && W_angle <= 0)
	//{
	//	W_angle += 3;
	//}
	//
	//if (!OpenGL::isKeyPressed('S') && W_angle >= 0)
	//{
	//	W_angle -= 3;
	//}
	//
	//if (!OpenGL::isKeyPressed('D') && left_right <= 0)
	//{
	//	left_right += 3;
	//}
	//
	//if (!OpenGL::isKeyPressed('A') && left_right >= 0)
	//{
	//	left_right -= 3;
	//}
}




void DrawQuad()
{
	double A[] = { -100,0 };
	double B[] = { -100,100 };
	double C[] = { 100,100 };
	double D[] = { 100,0 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}


ObjFile objModel,monkey;

ObjFile apple;
Texture appletex;


ObjFile air;
Texture airtex;

ObjFile prop;
Texture proptex;

Texture pol;

Texture monkeyTex;

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{

	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	
	


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   �������� �������� �� �����
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	frac.FshaderFileName = "shaders\\frac.frag"; //��� ����� ������������ �������
	frac.LoadShaderFromFile(); //��������� ������� �� �����
	frac.Compile(); //�����������

	cassini.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	cassini.FshaderFileName = "shaders\\cassini.frag"; //��� ����� ������������ �������
	cassini.LoadShaderFromFile(); //��������� ������� �� �����
	cassini.Compile(); //�����������
	

	s[0].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[0].FshaderFileName = "shaders\\light.frag"; //��� ����� ������������ �������
	s[0].LoadShaderFromFile(); //��������� ������� �� �����
	s[0].Compile(); //�����������

	s[1].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //��� ����� ������������ �������
	s[1].LoadShaderFromFile(); //��������� ������� �� �����
	s[1].Compile(); //�����������

	

	 //��� ��� ��� ������� ������ *.obj �����, ��� ��� ��� ��������� �� ���������� � ���������� �������, 
	 // ������������ �� ����� ����������, � ������������ ������ � *.obj_m
	//loadModel("models\\lpgun6.obj_m", &objModel);


	//�������� ������� ����������
	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\samoliotik_test.obj_m", &air);
	airtex.loadTextureFromFile("textures//samoliotik_UV_all.bmp");
	airtex.bindTexture();

	//���������
	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\samoliotik_prop.obj_m", &prop);
	proptex.loadTextureFromFile("textures//prop1.bmp");
	proptex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	pol.loadTextureFromFile("textures//textureauf.bmp");
	pol.bindTexture();
	//�������� ������� ������
	//glActiveTexture(GL_TEXTURE0);
	//loadModel("models\\�������.obj_m", &apple);
	//appletex.loadTextureFromFile("textures//Nice3.bmp");
	//appletex.bindTexture();


	//glActiveTexture(GL_TEXTURE0);
	//loadModel("models\\monkey.obj_m", &monkey);
	//monkeyTex.loadTextureFromFile("textures//tex.bmp");
	//monkeyTex.bindTexture();


	tick_n = GetTickCount();
	tick_o = tick_n;

	//rec.setSize(300, 100);
	//rec.setPosition(10, ogl->getHeight() - 100-10);
	//rec.setText("T - ���/���� �������\nL - ���/���� ���������\nF - ���� �� ������\nG - ������� ���� �� �����������\nG+��� ������� ���� �� ���������",0,0,0);

	
}

//����� 10-��� �������
double f10(double p1, double p2, double p3, double p4, double p5, double p6, double p7, double p8, double p9, double p10, double t)
{
	return pow(1 - t, 9) * p1 + 9 * t * pow(1 - t, 8) * p2 + 9 * pow(t, 2) * pow(1 - t, 7) * p3 + 9 * pow(t, 3) * pow(1 - t, 6) * p4 + 9 * pow(t, 4) * pow(1 - t, 5) * p5 + 9 * pow(t, 5) * pow(1 - t, 4) * p6 +
		9 * pow(t, 6) * pow(1 - t, 3) * p7 + 9 * pow(t, 7) * pow(1 - t, 2) * p8 + 9 * pow(t, 8) * pow(1 - t, 1) * p9 + pow(t, 9) * p10;
}

Vector3 f_arr10(double p1[3], double p2[3], double p3[3], double p4[3], double p5[3], double p6[3], double p7[3], double p8[3], double p9[3], double p10[3], double t)
{
	Vector3 P;
	P.setCoords(f10(p1[0], p2[0], p3[0], p4[0], p5[0], p6[0], p7[0], p8[0], p9[0], p10[0], t),
		f10(p1[1], p2[1], p3[1], p4[1], p5[1], p6[1], p7[1], p8[1], p9[1], p10[1], t),
		f10(p1[2], p2[2], p3[2], p4[2], p5[2], p6[2], p7[2], p8[2], p9[2], p10[2], t));
	return P;
}



double f(double p0, double p1, double p2, double p3, double t)
{
	return p0 * (1 - t) * (1 - t) * (1 - t) + 3 * p1 * t * (1 - t) * (1 - t) + p2 * t * t * (1 - t) + t * t * t * p3;
}

Vector3 f_arr(double p0[3], double p1[3], double p2[3], double p3[3], double t)
{
	Vector3 P;
	P.setCoords(f(p0[0], p1[0], p2[0], p3[0], t), f(p0[1], p1[1], p2[1], p3[1], t), f(p0[2], p1[2], p2[2], p3[2], t));
	return P;
}





Vector3 circle(double t_max)
{
	Vector3 P;
	P.setCoords(r*cos((2*3.141592*200*t_max)/180), r * sin((2 * 3.141592 * 200 * t_max) / 180), 0);
	return P;
}

double speed(double t_max)
{
	return t_max * 60000;
}

void circle()
{
	if (!stop)
	{
		if (!reverse)
		{
			t_max += delta / 5;
			if (t_max > 1)
				t_max = 0;
		}
		else
		{
			t_max -= delta / 5;
			if (t_max < 0)
				t_max = 1;
		}
	}

	double P1[] = { -38, 25, 8 };
	double P2[] = { 19, 29, -47 };
	double P3[] = { 53, -89, 59 };
	double P4[] = { -91, -49, 8 };
	double P5[] = { -53,-16, -22 };
	double P6[] = { -56, 29, -24 };
	double P7[] = { 14, -57, 3 };
	double P8[] = { 35, 54, 20 };
	double P9[] = { -92, -75, -37 };
	double P10[] = { 0, 0, 0 };



	// ����������� ���������� 1
	//double P1[] = { 1,0,-6 }; //���� �����
	//double P2[] = { 10, -7, -2};
	//double P3[] = { 20, 0, -2};
	//double P4[] = { -20, 15, 10};
	//double P5[] = { -25, -13, 20};
	//double P6[] = { 30, 35, 35 };
	//double P7[] = { -35, 26, 0 };
	//double P8[] = { 40, 0, 0 };
	//double P9[] = { -50, 50, 1 };
	//double P10[] = { 50, 1, 1 };


	//������ ������
	//glBegin(GL_LINE_STRIP);
	//for (double t = 0; t <= 1.0001; t += 0.01)
	//{
	//	double P[10];
	//	P[0] = f10(P1[0], P2[0], P3[0], P4[0], P5[0], P6[0], P7[0], P8[0], P9[0], P10[0], t);
	//	P[1] = f10(P1[1], P2[1], P3[1], P4[1], P5[1], P6[1], P7[1], P8[1], P9[1], P10[1], t);
	//	P[2] = f10(P1[2], P2[2], P3[2], P4[2], P5[2], P6[2], P7[2], P8[2], P9[2], P10[2], t);
	//	glVertex3dv(P); //������ ����� P
	//}
	//glEnd();




	//���������� ������ ����� 10-��� �������
	Vector3 P_old = f_arr10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, !reverse ? t_max - delta : t_max + delta);
	Vector3 P = f_arr10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, t_max);

	//����� 4-��� �������
	//Vector3 P_old = f_arr(P1, P2, P3, P4, !reverse ? t_max - delta : t_max + delta);
	//Vector3 P = f_arr(P1, P2, P3, P4, t_max);
	
	//��������������� ����������
	//Vector3 P_old = circle(!reverse ? t_max - delta : t_max + delta);
	//Vector3 P = circle(t_max);

	Vector3 vec = (P - P_old).normolize();

	Vector3 rotX(vec.X(), vec.Y(), 0);
	rotX = rotX.normolize();
	double cosx = Vector3(1, 0, 0).ScalProizv(rotX);
	Vector3 vecpr = Vector3(1, 0, 0).vectProisvedenie(rotX);

	double sinfiZ = vecpr.Z() / abs(vecpr.Z());
	double anglez = acos(cosx) * 180 / 3.141592 * sinfiZ;

	double angley = acos(vec.Z()) * 180 / 3.141592 + 90;


	glPushMatrix();
	
	glTranslated(P.X(), P.Y(), P.Z());
	glRotated(anglez, 0, 0, 1);
	glRotated(angley, 0, 1, 0);
	glRotated(left_right, 1, 0, 0);
	airtex.bindTexture();
	air.DrawObj();
	
	glPopMatrix();


	glPushMatrix();

	glTranslated(P.X(), P.Y(), P.Z());
	glRotated(anglez, 0, 0, 1);
	glRotated(angley, 0, 1, 0);
	glRotated(speed(t_max), 1, 0, 0);
	proptex.bindTexture();
	prop.DrawObj();

	glPopMatrix();
}



void flight_speed()
{
	//������ ��������
	if (flight_control == 1)
	{
		t_max_flight = 0.05;
	}

	if (flight_control == 2)
	{
		t_max_flight = 0.1;
	}

	if (flight_control == 3)
	{
		t_max_flight = 0.12;
	}
}

void test()
{
	if (!stop)
	{
		flight_speed();
		t_max += delta / 5;
		if (t_max > 1)
			t_max = 0;
	}
	else
	{
		t_max_flight = 0;
		t_max = 0;
	}
	

	MyKeyEvent();
	NonKeyEvent();


	vec0.setCoords(vec0.X() - t_max_flight * cos((rotY * 3.141592) / 180), vec0.Y() - t_max_flight * sin(-rotZ * 3.141592 / 180), vec0.Z() - t_max_flight * sin((rotY * 3.141592) / 180));


	if (rotX > 15)rotX = 15;
	if (rotX < -15)rotX = -15;
	if (rotZ > 25)rotZ = 25;
	if (rotZ < -25)rotZ = -25;


	//flight_control();

	glPushMatrix();
	glTranslated(vec0.X(), vec0.Y(), vec0.Z());

	glRotated(180, 1, 0, 0);

	
	glRotated(rotX, 1, 0, 0);
	glRotated(rotY, 0, 1, 0);
	glRotated(rotZ, 0, 0, 1);
	glRotated(-rotZ * flag_controlinvert, 1, 0, 0);
	glRotated(left_right * flag_controlinvert, 1, 0, 0);

	airtex.bindTexture();
	air.DrawObj();

	glRotated(speed(t_max), 1, 0, 0);
	proptex.bindTexture();
	prop.DrawObj();

	glPopMatrix();


	if (rotX > 0)rotX -= angleBackFrac;
	if (rotX < 0)rotX += angleBackFrac;
	//if (rotY > 0)rotY -= angleBackFrac;
	//if (rotY < 0)rotY += angleBackFrac;
	if (rotZ > 0)rotZ -= angleBackFrac;
	if (rotZ < 0)rotZ += angleBackFrac;
}






void Render(OpenGL *ogl)
{   
	
	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.2, 1. };
	GLfloat dif[] = { 0.5, 0.5, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//������ �����
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//������� ���  


	//





	s[0].UseShader();

	//�������� ���������� � ������.  ��� ���� - ���� ����� uniform ���������� �� �� �����. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//��� 2 - �������� �� ��������
	glUniform3fARB(location, light.pos.X(), light.pos.Y(),light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());

	//������ ��������
	//objModel.DrawObj();


	Shader::DontUseShaders();
	
	//������, ��� ��������
	//glPushMatrix();
	//	glTranslated(-5,15,0);
	//	//glScaled(-1.0,1.0,1.0);
	//	objModel.DrawObj();
	//glPopMatrix();


	//������� ���������� ������
	//s[1].UseShader();
	//int l = glGetUniformLocationARB(s[1].program, "Nice3");
	//glUniform1iARB(l, 0);

	//appletex.bindTexture();
	//apple.DrawObj();


		//s[1].UseShader();
	//int l = glGetUniformLocationARB(s[1].program, "Nice3");
	//glUniform1iARB(l, 0);

	//airtex.bindTexture();


	//t_max += delta;
	//if (t_max > 1) delta = -delta;
	//if (t_max < 0)  delta = -delta;
	//BF_40(delta, t_max);







	
	//1-�� ����
	//circle();

	//2-�� ���� �������� ���������� WASD ���������� ��������
	test();
	//MyKeyEvent();
	pol.bindTexture();
	DrawQuad();


	//glPushMatrix();
	//
	//prop.DrawObj();
	//
	//glPopMatrix();

	//glPushMatrix();
	//
	//glTranslated(0, -2, -0.5);
	//glRotated(90, 1, 0, 0);
	//glRotated(-90, 0, 1, 0);
	//
	//airtex.bindTexture();
	//air.DrawObj();
	//glPopMatrix();


	//��������

	//s[1].UseShader();
	//int l = glGetUniformLocationARB(s[1].program,"tex"); 
	//glUniform1iARB(l, 0);     //��� ��� ����� �� ��������� �������� ������� �� GL_TEXTURE0
	//glPushMatrix();
	//glTranslated(3, 3, 0);
	//glRotated(-90, 0, 0, 1);
	//monkeyTex.bindTexture();
	//monkey.DrawObj();
	//glPopMatrix();



	
	

	//////��������� ��������

	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		frac.UseShader();

		int location = glGetUniformLocationARB(frac.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());

		location = glGetUniformLocationARB(frac.program, "uOffset");
		glUniform2fARB(location, offsetX, offsetY);

		location = glGetUniformLocationARB(frac.program, "uZoom");
		glUniform1fARB(location, zoom);

		location = glGetUniformLocationARB(frac.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();

	}
	*/
	
	
	//////���� �������
	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		cassini.UseShader();

		int location = glGetUniformLocationARB(cassini.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());


		location = glGetUniformLocationARB(cassini.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();
	}

	*/

	
	
	

	
	Shader::DontUseShaders();

	
	
}   //����� ���� �������


bool gui_init = false;


//������ ���������, ��������� ����� �������� �������
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION); //������ �������� ������� ��������.
	//(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix(); //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ����
	glLoadIdentity(); //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1); //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW); //������������� �� �����-��� �������
	glPushMatrix(); //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity(); //���������� �� � ������

	glDisable(GL_LIGHTING);


	GuiTextRectangle rec; //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 250);
	rec.setPosition(10, ogl->getHeight() - 250 - 10);


	std::stringstream ss;
	ss << "���������� ��������� WASD" << std::endl;
	ss << "������� �������� <-, -> (���������)" << std::endl;
	ss << "C - ��������� �������� ������" << std::endl;
	ss << "�������� ������: " << flight_control << std::endl;
	ss << "M - ���������� �����" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	if (debug)
	{
		ss << "���������� �������: (" << vec0.X() << " , " << vec0.Y() << ", " << vec0.Z() << ")" << std::endl;
		ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
		ss << "���� �������� Oy: "<< rotY << std::endl;
		ss << "���� �������� Ox: " << left_right << std::endl;
	}
	if (!debug)
		ss.clear();

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION); //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	Shader::DontUseShaders();
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	//rec.setPosition(10, newH - 100 - 10);
}

