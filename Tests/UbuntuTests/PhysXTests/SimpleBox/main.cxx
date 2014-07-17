/*
 * This is a source code from this site:
 * http://mmmovania.blogspot.com/2011/05/simple-bouncing-box-physx3.html?utm_source=BP_recent
 * modified for compiling under Linux.
 *
 */

#include <iostream>
#include <cstdio>
#include <GL/freeglut.h>
#include <PxPhysicsAPI.h>
#include <PxExtensionsAPI.h>
#include <PxDefaultErrorCallback.h>
#include <PxDefaultAllocator.h>
#include <PxDefaultSimulationFilterShader.h>
#include <PxDefaultCpuDispatcher.h>
#include <PxShapeExt.h>
#include <PxMat33.h>
//#include <PxMat33Legacy.h>
#include <PxSimpleFactory.h>

using namespace std;
using namespace physx;

const int	WINDOW_WIDTH=1024,
WINDOW_HEIGHT=768;

static PxPhysics* gPhysicsSDK = NULL;
static PxFoundation* gFoundation=NULL;

static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;

static PxSimulationFilterShader gDefaultFilterShader=PxDefaultSimulationFilterShader;

PxScene* gScene = NULL;
PxRigidActor *box;


//for mouse dragging
int oldX=0, oldY=0;
float rX=15, rY=0;
float fps=0;
int startTime=0;
int totalFrames=0;
int state =1 ;
float dist=-5;

void SetOrthoForFont()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
	glScalef(1, -1, 1);
	glTranslatef(0, -WINDOW_HEIGHT, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ResetPerspectiveProjection()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void RenderSpacedBitmapString(
							  int x,
							  int y,
							  int spacing,
							  void *font,
							  char *string)
{
	char *c;
	int x1=x;
	for (c=string; *c != '\0'; c++) {
		glRasterPos2i(x1,y);
		glutBitmapCharacter(font, *c);
		x1 = x1 + glutBitmapWidth(font,*c) + spacing;
	}
}


void DrawAxes()
{
	//To prevent the view from disturbed on repaint
	//this push matrix call stores the current matrix state
	//and restores it once we are done with the arrow rendering
	glPushMatrix();
		glColor3f(0,0,1);
		glPushMatrix();
			glTranslatef(0,0, 0.8f);
			glutSolidCone(0.0325,0.2, 4,1);
			//Draw label
			glTranslatef(0,0.0625,0.225f);
			RenderSpacedBitmapString(0,0,0,GLUT_BITMAP_HELVETICA_10, "Z");
		glPopMatrix();
		glutSolidCone(0.0225,1, 4,1);

		glColor3f(1,0,0);
		glRotatef(90,0,1,0);
		glPushMatrix();
			glTranslatef(0,0,0.8f);
			glutSolidCone(0.0325,0.2, 4,1);
			//Draw label
			glTranslatef(0,0.0625,0.225f);
			RenderSpacedBitmapString(0,0,0,GLUT_BITMAP_HELVETICA_10, "X");
		glPopMatrix();
		glutSolidCone(0.0225,1, 4,1);

		glColor3f(0,1,0);
		glRotatef(90,-1,0,0);
		glPushMatrix();
			glTranslatef(0,0, 0.8f);
			glutSolidCone(0.0325,0.2, 4,1);
			//Draw label
			glTranslatef(0,0.0625,0.225f);
			RenderSpacedBitmapString(0,0,0,GLUT_BITMAP_HELVETICA_10, "Y");
		glPopMatrix();
		glutSolidCone(0.0225,1, 4,1);
	glPopMatrix();
}
void DrawGrid(int GRID_SIZE)
{
	glBegin(GL_LINES);
	glColor3f(0.75f, 0.75f, 0.75f);
	for(int i=-GRID_SIZE;i<=GRID_SIZE;i++)
	{
		glVertex3f((float)i,0,(float)-GRID_SIZE);
		glVertex3f((float)i,0,(float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE,0,(float)i);
		glVertex3f((float)GRID_SIZE,0,(float)i);
	}
	glEnd();
}



void StepPhysX(float dt)
{
	gScene->simulate(dt);

	//...perform useful work here using previous frame's state data
	while(!gScene->fetchResults() )
	{
		// do something useful
	}
}

void InitializePhysX() {
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    if(!gFoundation)
        cerr<<"PxCreateFoundation failed!"<<endl;

/*
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback, PxTolerancesScale() );
	*/

	bool recordMemoryAllocations = true;
    gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), recordMemoryAllocations);
    if(!gPhysicsSDK)
        cerr<<"PxCreatePhysics failed!"<<endl;

	if(gPhysicsSDK == NULL) {
		cerr<<"Error creating PhysX3 device."<<endl;
		cerr<<"Exiting..."<<endl;
		exit(1);
	}

    if(!PxInitExtensions(*gPhysicsSDK))
		cerr<< "PxInitExtensions failed!" <<endl;

	//PxExtensionVisualDebugger::connect(gPhysicsSDK->getPvdConnectionManager(),"localhost",5425, 10000, true);


	//Create the scene
	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity=PxVec3(0.0f, -9.8f, 0.0f);

     if(!sceneDesc.cpuDispatcher) {
        PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
        if(!mCpuDispatcher)
           cerr<<"PxDefaultCpuDispatcherCreate failed!"<<endl;
        sceneDesc.cpuDispatcher = mCpuDispatcher;
    }
 	if(!sceneDesc.filterShader)
        sceneDesc.filterShader  = gDefaultFilterShader;


	gScene = gPhysicsSDK->createScene(sceneDesc);
	if (!gScene)
        cerr<<"createScene failed!"<<endl;

	gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE,				 1.0);
	gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES,	1.0f);


	PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.5,0.5,0.5);

	//Create actors
    //1) Create ground plane
    PxReal d = 0.0f;
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f),PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));

	PxRigidStatic* plane = gPhysicsSDK->createRigidStatic(pose);
	if (!plane)
			cerr<<"create plane failed!"<<endl;

	PxShape* shape = plane->createShape(PxPlaneGeometry(), *mMaterial);
	if (!shape)
		cerr<<"create shape failed!"<<endl;
	gScene->addActor(*plane);


	//2) Create cube
	PxReal density = 1.0f;
	PxTransform transform(PxVec3(0.0f, 10.0f, 0.0f), PxQuat::createIdentity());
	PxVec3 dimensions(0.5,0.5,0.5);
	PxBoxGeometry geometry(dimensions);

	PxRigidDynamic *actor = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *mMaterial, density);
    actor->setAngularDamping(0.75);
    actor->setLinearVelocity(PxVec3(0,0,0));
	if (!actor)
		cerr<<"create actor failed!"<<endl;
	gScene->addActor(*actor);

	box = actor;
}

void getColumnMajor(PxMat33 m, PxVec3 t, float* mat) {
   mat[0] = m.column0[0];
   mat[1] = m.column0[1];
   mat[2] = m.column0[2];
   mat[3] = 0;

   mat[4] = m.column1[0];
   mat[5] = m.column1[1];
   mat[6] = m.column1[2];
   mat[7] = 0;

   mat[8] = m.column2[0];
   mat[9] = m.column2[1];
   mat[10] = m.column2[2];
   mat[11] = 0;

   mat[12] = t[0];
   mat[13] = t[1];
   mat[14] = t[2];
   mat[15] = 1;
}

void DrawBox(PxMat44& m, PxShape* pShape) {
/*
	PxTransform pT = PxShapeExt::getGlobalPose(*pShape);
	PxBoxGeometry bg;
	pShape->getBoxGeometry(bg);
	PxMat33 m = PxMat33Legacy(pT.q ).toPxMat33();
    float mat[16];
	getColumnMajor(m,pT.p, mat);
	glPushMatrix();
		glMultMatrixf(mat);
		glutSolidCube(bg.halfExtents.x*2);
    glPopMatrix();
    */

    PxBoxGeometry bg;
	pShape->getBoxGeometry(bg);



    // set to OpenGL
    glPushMatrix();
        // PxMat44::front() returns a pointer to the first matrix element
        glMultMatrixf(m.front());

        // draw model
        glutSolidCube(bg.halfExtents.x*2);
    glPopMatrix();
}
void DrawShape(PxMat44& m, PxShape* shape)
{
	PxGeometryType::Enum type = shape->getGeometryType();
    switch(type)
    {
		case PxGeometryType::eBOX:
			DrawBox(m, shape);
		break;
    }
}

void DrawActor(PxRigidActor* actor)
{
	PxU32 nShapes = actor->getNbShapes();
    PxShape** shapes=new PxShape*[nShapes];

	actor->getShapes(shapes, nShapes);

    while (nShapes--)
    {
        // retrieve world space transform of rigid body
        PxTransform t = actor->getGlobalPose();

        // convert to matrix form
        PxMat44 m = PxMat44(t);
        DrawShape(m, shapes[nShapes]);
    }
	delete [] shapes;
}

void RenderActors()
{
    // Render all the actors in the scene

	DrawActor(box);
}




void ShutdownPhysX() {
	gScene->removeActor(*box);
	gScene->release();
	box->release();
	gPhysicsSDK->release();
}

void InitGL() {
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat ambient[4]={0.25f,0.25f,0.25f,0.25f};
	GLfloat diffuse[4]={1,1,1,1};
	GLfloat mat_diffuse[4]={0.85f,0,0,0};

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

	glDisable(GL_LIGHTING);
}

void OnReshape(int nw, int nh) {
	glViewport(0,0,nw, nh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)nw / (GLfloat)nh, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

char buffer[256];
float old = 0, current = 0;


void OnRender() {
	//Calculate fps
	totalFrames++;
	old = current;
	current = (float)glutGet(GLUT_ELAPSED_TIME);

	if((current-startTime)>1000)
	{
		float elapsedTime = float(current-startTime);
		fps = ((totalFrames * 1000.0f)/ elapsedTime) ;
		startTime = current;
		totalFrames=0;
	}

	float dt = float(current-old)/1000.0f;
	sprintf(buffer, "FPS: %3.2f dt: %3.3f",fps, dt);

	//Update PhysX
    if (gScene)
    {
        StepPhysX(dt);
    }


	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0,0,dist);
	glRotatef(rX,1,0,0);
	glRotatef(rY,0,1,0);

	//Draw the grid and axes
	DrawAxes();
	DrawGrid(10);

	glEnable(GL_LIGHTING);
		RenderActors();
	glDisable(GL_LIGHTING);

	SetOrthoForFont();
		glColor3f(1,1,1);
		//Show the fps
		RenderSpacedBitmapString(20,20,0,GLUT_BITMAP_HELVETICA_12,buffer);

	ResetPerspectiveProjection();

	glutSwapBuffers();
}

void OnShutdown() {
	ShutdownPhysX();
}


void Mouse(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;
}

void Motion(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else
	{
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

void OnIdle() {
	glutPostRedisplay();
}

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("GLUT PhysX v 3.3.1 Demo - Simple Box");

	glutDisplayFunc(OnRender);
	glutIdleFunc(OnIdle);
	glutReshapeFunc(OnReshape);
    glutCloseFunc(OnShutdown);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	InitGL();
	InitializePhysX();

	glutMainLoop();

	return 0;
}
