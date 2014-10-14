#define _USE_MATH_DEFINES 
#include <iostream> 

#include <cassert>

#define CHECK_GL_ERROR assert(glGetError() == GL_NO_ERROR);

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/freeglut.h>

#ifdef _WIN32  
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Common_x86.lib") 
#pragma comment(lib, "PhysX3Cooking_x86.lib") 
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PxTask.lib") 

#endif

#include <vector>
 
#include <glm/glm.hpp>

using namespace std;
const int width = 800, height = 600;

int selected_index =0;// -1;

int oldX=0, oldY=0;
float rX=27.79f, rY=45;
int state =1 ;
float tx, ty, tz = -8;

const int GRID_SIZE=10;
 
 
float currentTime = 0; 

const int total_points = 16;
 
float frameTime =0 ;

float startTime =0, fps=0 ;
int totalFrames=0;

using namespace std;

GLuint textureID; 

GLfloat mat_white[4]={1.0f,1.0f,1.0f,1};
GLfloat mat_grey[4]={0.25f,0.25f,0.25f,1};
GLfloat mat_brown[4]={0.5f,0.3f,0.0f,1};
 

#include "PhysXCloth.h"
PhysXSimulator simulator;

#include <sstream>

GLint viewport[4];
GLdouble MV[16];
GLdouble P[16];


PxVec3 Up=PxVec3(0,1,0), Right, viewDir;

void OnMouseDown(int button, int s, int x, int y)
{
	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1; 

	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y; 

		if(state == 1) {
			int winY = (viewport[3] - y);  
			int winX = x;
			float winZ = 0; 

			glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
  

			double objX=0, objY=0, objZ=0;
			gluUnProject(winX, winY, winZ,  MV,  P, viewport, &objX, &objY, &objZ);
			PxVec3 pt(objX,objY, objZ); 
			//cout << "Pt: " << objX <<" " << objY << " " << objZ << endl;
			selected_index = simulator.onPickingClothVertex(pt);
		}
	}

	if(s==GLUT_UP) {   
		selected_index = -1;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

}

void OnMouseMove(int x, int y)
{
	if(selected_index == -1) {
		if (state == 0)
			tz *= (1 + (y - oldY)/60.0f); 
		else if (state == 2) {
			tx -= (x - oldX)/50.0f; 
			ty += (y - oldY)/50.0f; 
		}
		else
		{
			rY += (x - oldX)/5.0f; 
			rX += (y - oldY)/5.0f; 
		} 
	} else {
		float delta = 10.0f/abs(tz);
		float valX = (x - oldX)/delta; 
		float valY = (y - oldY )/delta; 
		if(abs(valX)>abs(valY))
			glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
		else 
			glutSetCursor(GLUT_CURSOR_UP_DOWN);
				 
		PxVec3& pos = simulator.getPickedClothParticlePosition(selected_index); 
		pos += Right*valX + Up*valY;
		simulator.updateClothParticlePosition(selected_index, pos); 
	}

	
	oldX = x; 
	oldY = y; 

	glutPostRedisplay();
}

//procedural texture generator 
void GenerateTexture(int width, int height) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLubyte* pData = new GLubyte [width*height*4];

	int dx = width/6;
	int dy = height/6;
	int hwidth = width/2;
	int hheight = height/2;
	int count = 0;
	//first draw horizontal lines
	for(int j=0;j<height;++j) {
		for(int i=0;i<width;i++) { 
			pData[count++]= 255;
			pData[count++]= (i<hwidth && j<height  || i>hwidth && j>height )?255:0;
			pData[count++]= (i<width  && j<hheight || i>width  && j>hheight)?0:255;
			pData[count++]= 255;
		}
	}

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA, GL_UNSIGNED_BYTE, pData);

	delete [] pData;

}

void DrawGrid()
{
	glBegin(GL_LINES);
	glColor3f(0.5f, 0.5f, 0.5f);
	for(int i=-GRID_SIZE;i<=GRID_SIZE;i++)
	{
		glVertex3f((float)i,0,(float)-GRID_SIZE);
		glVertex3f((float)i,0,(float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE,0,(float)i);
		glVertex3f((float)GRID_SIZE,0,(float)i);
	}
	glEnd();
} 

void InitGL() { 
	CHECK_GL_ERROR
	glClearColor(1,1,1,1);
	startTime = (float)glutGet(GLUT_ELAPSED_TIME);
	  
	glEnable(GL_DEPTH_TEST);  
	glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_BACK, GL_LINE);
	glEnable(GL_LIGHT0); //key light 
	glEnable(GL_LIGHT1); //fill light
	glEnable(GL_LIGHT2); //rim light
	glEnable(GL_LIGHTING); 
	glEnable(GL_NORMALIZE); 
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5);

	GLfloat ambient[4]={0.05f,0.05f,0.05f,0.05f};
	GLfloat mat_key[4]={0.4f,0.4f,0.25f,0.33f};
	GLfloat mat_fill[4]={0.35f,0.35f,0.45f,0.33f};

	
	//Rim/back light setup
	GLfloat pos[4] = {-0.1f,0.65f,-1.0f,0}; //back(rim) light position
	 
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, mat_grey);
	glLightfv(GL_LIGHT2, GL_SPECULAR, mat_grey); 
	glLightfv(GL_LIGHT2, GL_POSITION, pos);

	//key light setup
	pos[0] = -1;
	pos[1] = 1;
	pos[2] = 1; 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, mat_key); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, mat_key); 
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	 
	//fill light setup
	pos[0]=-pos[0]; 

	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, mat_fill); 
	glLightfv(GL_LIGHT1, GL_SPECULAR, mat_fill);  
	
	GLfloat ns = 128;

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_white); 	 
	glMaterialf(GL_FRONT, GL_SHININESS, ns); 

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);  
	glDisable(GL_LIGHTING); 
	 
	GenerateTexture(128,128); 
	 
	CHECK_GL_ERROR

	//initialize physx simulator
	simulator.init();
	simulator.createStaticActors();
	simulator.createCloth(3.5f,3.5f,4.6f,0.05f); 
}

void OnReshape(int nw, int nh) {
	glViewport(0,0,nw, nh);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)nw / (GLfloat)nh, 0.1f, 1000.0f); 

	glGetDoublev(GL_PROJECTION_MATRIX, P);

	glMatrixMode(GL_MODELVIEW);
}
 
void DrawAxes() {
	glBegin(GL_LINES);
	glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(50,0,0);
	glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,50,0);
	glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,50);
	glEnd();
}
void DrawPlane() {
	glBegin(GL_QUADS); 
	glNormal3f(0,1,0);
	glVertex3f(-100, 0,-100);
	glVertex3f(-100, 0, 100);			
	glVertex3f( 100, 0, 100);
	glVertex3f( 100, 0,-100);
	glEnd();
} 

void DrawSimulatedClothShadow() { 
	glEnableClientState(GL_VERTEX_ARRAY);  
	glVertexPointer(3, GL_FLOAT, sizeof(PxVec3), &(simulator.pos[0].x)); 
	glDrawElements(GL_TRIANGLES, simulator.numIndices, GL_UNSIGNED_INT, &simulator.indices[0]); 
	glDisableClientState(GL_VERTEX_ARRAY); 
}

void DrawSimulatedCloth(bool bShowParticles=false) { 
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(PxVec3), &(simulator.pos[0].x));
	 
	glNormalPointer(GL_FLOAT, sizeof(PxVec3), &(simulator.normal[0].x));
	glTexCoordPointer(2, GL_FLOAT,sizeof(glm::vec2), &(simulator.uvs[0].x));

	glDrawElements(GL_TRIANGLES, simulator.numIndices, GL_UNSIGNED_INT, &simulator.indices[0]);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	 
	if(bShowParticles) {
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glBegin(GL_POINTS);
	
		for(size_t i=0;i<simulator.numVertices;++i) {
			if(selected_index == i)
				glColor3f(0,1,0);
			else
				glColor3f(1,0,0);
			glVertex3fv(&simulator.pos[i].x);
		}
		glEnd(); 
	}
}
 
void DrawTable() { 
	//table top
	
	glPushMatrix();
		glTranslatef(0,1,0);	
		glScalef(2,0.2f,2);
		glutSolidCube(1);
	glPopMatrix();
	
	//the table's base
	glPushMatrix();
		glTranslatef(0,0.5f,0);
		glScalef(0.5f,0.975f,0.5f);
		glutSolidCube(1);
	glPopMatrix();
}
  

void OnRender() {  
	
	simulator.step(); 
	simulator.updateCloth();
	 
	

	CHECK_GL_ERROR

	float newTime = (float) glutGet(GLUT_ELAPSED_TIME);
	frameTime = newTime-currentTime;
	currentTime = newTime;
	//accumulator += frameTime;

	 

	++totalFrames;
	if((newTime-startTime)>1000)
	{
		float elapsedTime = (newTime-startTime);
		fps = (totalFrames/ elapsedTime)*1000 ;
		startTime = newTime;
		totalFrames=0;
	}

	std::stringstream str;
    str<<"FPS: " << fps << ", Frame time (GLUT): " << frameTime << " msecs.";

	glutSetWindowTitle(str.str().c_str()); 

	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(tx,ty,tz);
	glRotatef(rX,1,0,0);
	glRotatef(rY,0,1,0);

	glGetDoublev(GL_MODELVIEW_MATRIX, MV);
	viewDir.x = (float)MV[2];
	viewDir.y = (float)MV[6];
	viewDir.z = (float)MV[10];
	Right = viewDir.cross(Up);

	CHECK_GL_ERROR

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	 
		//draw ground plane
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_white); 
		DrawPlane(); 

		//draw table
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_brown); 
		DrawTable(); 
		  
		//draw cloth  
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_white); 
		glEnable(GL_TEXTURE_2D);
		 
		DrawSimulatedCloth();  

	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_LIGHTING);

	CHECK_GL_ERROR
		 
	// simple XZ planar shadows 
	// added a little bias to prevent shadow acne
	static GLfloat ShadowMat[]={ 1,0,0,0, 
		0,0,0,0, 
		0,0,1,0, 
		0.0f,0.001f,0.0f,1 };

	glPushMatrix();	
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		  
		glEnable(GL_CULL_FACE);
		glMultMatrixf(ShadowMat);		

		glColor4f(0.25f, 0.25f, 0.275f,0.2f); 
		DrawTable();    
		
		glColor4f(0.25f, 0.25f, 0.275f,0.8f); 
		DrawSimulatedClothShadow();
		
		glDisable(GL_CULL_FACE);		
		glDisable(GL_BLEND);				
		glDepthMask(GL_TRUE);

	glPopMatrix();
	
	
	glDisable(GL_DEPTH_TEST);
	 
	//draw the axes in the bottom left corner
	glPushMatrix();
		glLoadIdentity(); 
		glTranslatef(50, 50, 0);		
		glRotatef(rX,1,0,0);
		glRotatef(rY,0,1,0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glOrtho(0,width,0,height,-50,50);
				DrawAxes();  
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
 
	 
	glutSwapBuffers();
}

void OnShutdown() {   
	simulator.shutdown();

	glDeleteTextures(1, &textureID); 
}

void OnIdle() { 
	
	glutPostRedisplay();  
}  

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Simple Demo Test");

	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnReshape);
	glutIdleFunc(OnIdle);

	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove); 
	glutCloseFunc(OnShutdown); 
	  

	InitGL();

	glutMainLoop();

	return 0;
}
