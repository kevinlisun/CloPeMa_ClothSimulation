#include <GL/glew.h>
#include <iostream> 
#include "stdio.h"


#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/freeglut.h>
 
#ifdef _WIN32 
#pragma comment(lib, "glew32.lib")
#endif

#include <vector>

std::vector<GLuint> textures;				//all textures
#include <glm/glm.hpp>

using namespace std;
const int width = 800, height = 600;

int selected_index = -1;

int oldX=0, oldY=0;
float rX=27.79f, rY=0;
int state =1 ;
float tx, ty, tz = -7;

const int GRID_SIZE=10;

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

char info[MAX_PATH]={0};

float timeStep =  1/60.0f;
float currentTime = 0;
double accumulator = timeStep;

const int total_points = 16;

//LARGE_INTEGER frequency;        // ticks per second
//LARGE_INTEGER t1, t2;           // ticks
//double frameTimeQP=0;
float frameTime =0 ;

float startTime =0, fps=0 ;
int totalFrames=0;

using namespace std;
 
GLuint textureID;
std::string mesh_filename = "media/roundTable.obj";

#include <tiny_obj_loader.h>

std::vector<tinyobj::shape_t> tableMesh;
 
GLfloat mat_white[4]={1.0f,1.0f,1.0f,1};
GLfloat mat_brown[4]={0.5f,0.3f,0.0f,1};

void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y; 
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1; 
}

void OnMouseMove(int x, int y)
{
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
			pData[count++]= (i<width  && j<hheight || i>width  && j>hheight)?0:255;
			pData[count++]= (i<hwidth && j<height  || i>hwidth && j>height )?255:0;
			pData[count++]= 255;
			pData[count++]= 255;
		}
	}
	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_BGRA, GL_UNSIGNED_BYTE, pData);
	
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
	startTime = (float)glutGet(GLUT_ELAPSED_TIME);
	// get ticks per second
	//QueryPerformanceFrequency(&frequency);

	// start timer
	//QueryPerformanceCounter(&t1);

	glEnable(GL_DEPTH_TEST);  
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING); 
	glEnable(GL_NORMALIZE);

	GLfloat ambient[4]={0.125f,0.125f,0.125f,0.125f};
	  

	GLfloat mat_key[4]={1.0f,1.0f,0.9f,1};
	GLfloat mat_fill[4]={0.9f,0.9f,1.0f,1};

	GLfloat pos[4] = {0,0,1,0};
	GLfloat ns = 500;

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, mat_key);
	glLightfv(GL_LIGHT0, GL_SPECULAR, mat_white); 
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	pos[0]=1;
	pos[1]=1;
	pos[2]=-pos[2];
	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	 
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, mat_fill); 
	glLightfv(GL_LIGHT1, GL_SPECULAR, mat_white); 

	
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_white); 
	glMaterialfv(GL_FRONT, GL_SHININESS, &ns);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); 
	glDisable(GL_LIGHTING); 
	 
	GenerateTexture(128,128);

	cout << "Loading table mesh" << endl;
	 
	//get the mesh path for loading of textures	
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/")+1); 
	std::string err = tinyobj::LoadObj(tableMesh, mesh_filename.c_str(), mesh_path.c_str());

        cout<<"the mesh file name is "<<mesh_filename.c_str()<<endl;
        cout<<"the mesh path is "<<mesh_path.c_str()<<endl;

	if (!err.empty()) {
		std::cerr << err << std::endl;
		exit(1);
	}

	std::cout << "No. of meshes : " << tableMesh.size() << std::endl; 
}

void OnReshape(int nw, int nh) {
	glViewport(0,0,nw, nh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)nw / (GLfloat)nh, 0.1f, 1000.0f); 
	glMatrixMode(GL_MODELVIEW);
}


void DrawTable( ) {  
	glEnableClientState(GL_VERTEX_ARRAY);	 
	glEnableClientState(GL_NORMAL_ARRAY);   

	for(size_t i=0;i<tableMesh.size(); ++i) { 
		glVertexPointer(3, GL_FLOAT, 0, &(tableMesh[i].mesh.positions[0]));
		glNormalPointer(GL_FLOAT, 0, &(tableMesh[i].mesh.normals[0])); 
		 
		if(!tableMesh[i].material.diffuse_texname.empty())
			glDrawElements(GL_TRIANGLES, tableMesh[i].mesh.indices.size(), GL_UNSIGNED_INT, &(tableMesh[i].mesh.indices[0]));
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);  
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
void DrawCloth(float halfXSize = 2.0f, float y=2.5f, float halfZSize=2.0f) {
	glBegin(GL_QUADS); 
		glNormal3f(0,1,0);
		glTexCoord2f(0,0);		glVertex3f(-2, y,-2);
		glTexCoord2f(0,9.5);	glVertex3f(-2, y, 2);			
		glTexCoord2f(9.5,9.5);	glVertex3f( 2, y, 2);
		glTexCoord2f(9.5,0);	glVertex3f( 2, y,-2);
	glEnd();
}

void OnRender() { 
	float newTime = (float) glutGet(GLUT_ELAPSED_TIME);
	frameTime = newTime-currentTime;
	currentTime = newTime;
	//accumulator += frameTime;

	//Using high res. counter
	//QueryPerformanceCounter(&t2);
	// compute and print the elapsed time in millisec
	//frameTimeQP = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	//t1=t2;
	//accumulator += frameTimeQP;

	++totalFrames;
	if((newTime-startTime)>1000)
	{
		float elapsedTime = (newTime-startTime);
		fps = (totalFrames/ elapsedTime)*1000 ;
		startTime = newTime;
		totalFrames=0;
	}

	sprintf(info, "FPS: %3.2f, Frame time (GLUT): %3.4f msecs", fps, frameTime);
	glutSetWindowTitle(info);

	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	 
	glTranslatef(tx,ty,tz);
	glRotatef(rX,1,0,0);
	glRotatef(rY,0,1,0);

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
		
		DrawCloth();

		glDisable(GL_TEXTURE_2D); 

	glDisable(GL_LIGHTING);
	

	// simple XZ planar shadows
	// a simple XZ plane ortho graphic projector
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
		glColor4f(0.25f, 0.25f, 0.275f,0.5f); 
		DrawTable();  
		DrawCloth(); 
	glPopMatrix();
	glDepthMask(GL_TRUE);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
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
				glOrtho(0,width,0,height,-100,100);
				DrawAxes();  
	 	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glutSwapBuffers();
}

void OnShutdown() {   
	glDeleteTextures(1, &textureID);
	tableMesh.clear();
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

	
	glewInit();

	InitGL();

	glutMainLoop();

	return 0;
}
