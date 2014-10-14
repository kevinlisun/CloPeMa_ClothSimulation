#ifndef PHYSX_CLOTH_INCLUDED
#define PHYSX_CLOTH_INCLUDED
#pragma once

#include <PxPhysicsAPI.h>
#include <extensions/PxExtensionsAPI.h>
#include <PxToolkit.h>

#include <iostream>
#include <vector>

using namespace physx;

static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader=PxDefaultSimulationFilterShader;

class PhysXSimulator {

public:
	PhysXSimulator() {

	}

	~PhysXSimulator() {}

	void init() {
		foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
		if(!foundation)
			cerr<<"PxCreateFoundation failed!"<<endl;

		pPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation,  PxTolerancesScale());
		if(pPhysicsSDK == NULL) {
			cerr<<"Error creating PhysX3 device."<<endl;
			cerr<<"Exiting..."<<endl;
			exit(1);
		}

		if(!PxInitExtensions(*pPhysicsSDK))
			cerr<< "PxInitExtensions failed!" <<endl;

		//Create the scene
		PxSceneDesc sceneDesc(pPhysicsSDK->getTolerancesScale());
		sceneDesc.gravity=PxVec3(0.0f, -9.8f, 0.0f);

		if(!sceneDesc.cpuDispatcher) {
			PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
			if(!mCpuDispatcher)
				cerr<<"PxDefaultCpuDispatcherCreate failed!"<<endl;
			sceneDesc.cpuDispatcher = mCpuDispatcher;
		}

		if(!sceneDesc.filterShader)
			sceneDesc.filterShader  = gDefaultFilterShader;

		pScene = pPhysicsSDK->createScene(sceneDesc);
		if (!pScene)
			cerr<<"createScene failed!"<<endl;

		pScene->setVisualizationParameter(PxVisualizationParameter::eSCALE,				 1.0);
		pScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES,	1.0f);
		pScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_EDGES,	1.0f);
		pScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES,	1.0f);

		//gScene->setVisualizationParameter(PxVisualizationParameter::eDEFORMABLE_MESH, 1.0f);
		//gScene->setVisualizationParameter(PxVisualizationParameter::eDEFORMABLE_SELFCOLLISIONS, 1.0f);
		//gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.0f);
		//gScene->setVisualizationParameter(PxVisualizationParameter::eDEFORMABLE_SHAPES, 1.0f);

		PxMaterial* mMaterial = pPhysicsSDK->createMaterial(0.5,0.5,0.5);

		//Create actors
		//1) Create ground plane
		PxReal d = 0.0f;
		PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f),PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));

		PxRigidStatic* plane = pPhysicsSDK->createRigidStatic(pose);
		if (!plane)
			cerr<<"create plane failed!"<<endl;

		PxShape* shape = plane->createShape(PxPlaneGeometry(), *mMaterial);
		shape->setContactOffset(0.0025f);
		if (!shape)
			cerr<<"create shape failed!"<<endl;
		pScene->addActor(*plane);


	}

	void shutdown() {
		if(pScene) {
			PxActorTypeFlags desiredTypes = PxActorTypeFlag::eRIGID_STATIC|PxActorTypeFlag::eRIGID_DYNAMIC;
			PxU32 count = pScene->getNbActors(desiredTypes);
			PxActor** buffer = new PxActor*[count];
			pScene->getActors(desiredTypes, buffer, count);

			while (count)
			{
				buffer[--count]->release();
			}
			delete [] buffer;

			pScene->release();
		}
		pPhysicsSDK->release();
	}

	void step() {
		pScene->simulate(timeStep);

		//...perform useful work here using previous frame's state data
		while(!pScene->fetchResults()) {
		}
	}

	void updateCloth() {
		//update the cloth data
		PxClothParticleData* pData = pCloth->lockParticleData();
		PxClothParticle* pParticles = const_cast<PxClothParticle*>(pData->particles);

		//update the positions
		for(size_t i=0;i<pos.size();i++) {
			pos[i] = pParticles[i].pos;
		}
		pData->unlock();

		//update normals
		for(size_t i=0;i<indices.size();i+=3) {
			PxVec3 p1 = pos[indices[i]];
			PxVec3 p2 = pos[indices[i+1]];
			PxVec3 p3 = pos[indices[i+2]];
			PxVec3 n  = (p2-p1).cross(p3-p1);

			normal[indices[i]]    += n;
			normal[indices[i+1]]  += n;
			normal[indices[i+2]]  += n;
		}

		for(size_t i=0;i<normal.size();i++) {
			PxVec3& n  = normal[i];
			n= n.getNormalized();
 		}
	}
	void createCloth(float width, float height, float ypos, float spacing=0.2f) {

		//get required parameters
		float hWidth = width / 2.0f;
		float hHeight = height / 2.0f;
		int numX = (int)((width / spacing) + 1);
		int numY = (int)((height / spacing) + 1);

		std::cout << "Total cloth vertices on X Axis: " << numX << std::endl;
		std::cout << "Total cloth vertices on Z Axis: " << numY << std::endl;

		//fill cloth mesh descriptor
		PxClothMeshDesc meshDesc;
		meshDesc.setToDefault();
		meshDesc.points.count= (numX+1) * (numY+1);
		meshDesc.triangles.count= numX*numY*2;
		meshDesc.points.stride= sizeof(PxVec3);
		meshDesc.triangles.stride= 3*sizeof(PxU32);
		meshDesc.points.data= (PxVec3*)malloc(sizeof(PxVec3)*meshDesc.points.count);
		meshDesc.triangles.data= (PxU32*)malloc(sizeof(PxU32)*meshDesc.triangles.count*3);


		//fill vertex positions and uvs
		int i,j;
		PxVec3 *p = (PxVec3*)meshDesc.points.data;

		pos.resize(meshDesc.points.count);
		normal.resize(meshDesc.points.count);
		indices.resize(meshDesc.triangles.count*3);

		for (i = 0; i <= numY; i++) {
			for (j = 0; j <= numX; j++) {
				float u = static_cast<float>(j)/numX;
				float v = static_cast<float>(i)/numY;
				uvs.push_back(glm::vec2(u*9.5f,v*9.5f));

				p->x=spacing*j-hWidth;
				p->y=ypos;
				p->z=spacing*i-hHeight;
				p++;
			}
		}
		memcpy(&pos[0].x, (meshDesc.points.data), sizeof(PxVec3)*meshDesc.points.count);

		//fill indices
		//Fill the topology
		PxU32 *id = (PxU32*)meshDesc.triangles.data;
		for (i = 0; i < numY; i++) {
			for (j = 0; j < numX; j++) {
				PxU32 i0 = i * (numX+1) + j;
				PxU32 i1 = i0 + 1;
				PxU32 i2 = i0 + (numX+1);
				PxU32 i3 = i2 + 1;
				if ((j+i)%2) {
					*id++ = i0; *id++ = i2; *id++ = i1;
					*id++ = i1; *id++ = i2; *id++ = i3;
				} else {
					*id++ = i0; *id++ = i2; *id++ = i3;
					*id++ = i0; *id++ = i3; *id++ = i1;
				}
			}
		}

		memcpy(&indices[0], meshDesc.triangles.data, sizeof(PxU32)*meshDesc.triangles.count*3);
		numIndices = meshDesc.triangles.count*3;

		//Make sure everything is fine so far
		if(!(meshDesc.isValid()))
			cerr<<"Mesh invalid."<<endl;

		//Start cooking of fibres
		PxCookingParams cp(pPhysicsSDK->getTolerancesScale());
		PxCooking* cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, cp);

		PxClothFabricCooker cooker(meshDesc, pScene->getGravity());
		PxDefaultMemoryOutputStream wb;
		cooker.save(wb, false);

		PxDefaultMemoryInputData rb(wb.getData(), wb.getSize());
		PxClothFabric* fabric=pPhysicsSDK->createClothFabric(rb);

		PxTransform tr;
		tr.p = PxVec3(0,0,0); tr.q = PxQuat::createIdentity();

		PxClothParticle* points=(PxClothParticle*)malloc(sizeof(PxClothParticle)*meshDesc.points.count);
		p = (PxVec3*)meshDesc.points.data;
		for(size_t i=0;i<meshDesc.points.count;i++) {
			points[i].pos = *p;
			// Fixing the top corner points
			if(i<=5 || (i<=numX+2 & i>=numX-2))
<<<<<<< HEAD
				points[i].invWeight =0;
=======
				points[i].invWeight = 1.f;
>>>>>>> b264f3b7cacaf5ac3216450d3013794a673b4225
			else
				points[i].invWeight = 1.f;
			p++;
		}
		pCloth = pPhysicsSDK->createCloth(tr,*fabric,points,  PxClothFlag::eSWEPT_CONTACT|PxClothFlag::eSCENE_COLLISION);

		if(pCloth) {

			pCloth->setSolverFrequency(500.0f);

			PxClothCollisionSphere s;
			s.pos=PxVec3(0,2,0);
			s.radius=1;


			PxClothCollisionPlane p;
			p.normal = PxVec3(0.0f, 1.0f, 0.0f);
			p.distance = -0.01f;

			PxU32 convexMask = 1; // Convex references the first plane only

			//collision with floor
			pCloth->addCollisionPlane(p);
			pCloth->addCollisionConvex(convexMask);
			//pCloth->addCollisionSphere(s);

			/*
			//collision with the table top
			static PxVec3 t_vertices[8] = { PxVec3(-1, 1-0.1, -1),
									 PxVec3(-1, 1-0.1,  1),
									 PxVec3(-1, 1+0.1, -1),
									 PxVec3(-1, 1+0.1,  1),
									 PxVec3( 1, 1-0.1, -1),
									 PxVec3( 1, 1-0.1,  1),
									 PxVec3( 1, 1+0.1, -1),
									 PxVec3( 1, 1+0.1,  1) };

			static GLushort t_indices[]={0,1,2, 2,1,3,
									   4,6,5, 6,7,5,
									   2,3,6, 6,3,7,
									   0,2,4, 4,2,6,
									   1,5,3, 3,5,7
										};
			static int total = sizeof(t_indices)/sizeof(t_indices[0]);
			PxClothCollisionTriangle t;
			for(int i=0;i<total;i+=3) {
				t.vertex0 = t_vertices[i];
				t.vertex1 = t_vertices[i+1];
				t.vertex2 = t_vertices[i+2];
				pCloth->addCollisionTriangle(t);
			}*/


			pCloth->setDampingCoefficient(PxVec3(0.25f));
			pCloth->setSelfCollisionDistance(0.05f);
			pCloth->setSelfCollisionStiffness(0.25f);

            // setting constraint types
			pCloth->setStretchConfig(PxClothFabricPhaseType::eVERTICAL, PxClothStretchConfig(1.0f));
			pCloth->setStretchConfig(PxClothFabricPhaseType::eHORIZONTAL, PxClothStretchConfig(1.0f));
			pCloth->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(1.0f));
			pCloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(1.0f));

			PxClothStretchConfig stretchConfig;
			stretchConfig.stiffness = 2.0f;
			stretchConfig.stiffnessMultiplier = 0.5f;
			stretchConfig.compressionLimit = 0.9f;
			stretchConfig.stretchLimit = 1.1f;
			pCloth->setStretchConfig(PxClothFabricPhaseType::eVERTICAL, stretchConfig);

            pCloth->setFrictionCoefficient(0.5f);

			pCloth->setRestOffset(0.0025f);
			pScene->addActor(*pCloth);
		}
	}

	void createStaticActors() {
		//1) Create ground plane
		PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f),PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));

		PxRigidStatic* plane = pPhysicsSDK->createRigidStatic(pose);
		if (!plane)
			cerr<<"create plane failed!"<<endl;

		pMaterial = pPhysicsSDK->createMaterial(0.5,0.5,0.5);

		PxShape* shape = plane->createShape(PxPlaneGeometry(), *pMaterial);
		if (!shape)
			cerr<<"create shape failed!"<<endl;
		pScene->addActor(*plane);


		//create collidor for the table
		createCollider();
	}

private:
	void createCollider()
	{
		//2) Create table top
		/*
		PxBodyDesc bodyDesc;
		NxActorDesc actorDesc;
		NxBoxShapeDesc boxDesc;

		bodyDesc.angularDamping	= 0.75f;
		bodyDesc.linearVelocity = NxVec3(0,0,0);

		boxDesc.dimensions = NxVec3(1, 0.1f, 1);

		actorDesc.shapes.pushBack(&boxDesc);
		actorDesc.body			= &bodyDesc;
		actorDesc.density		= 1.0f;
		actorDesc.globalPose.t  = NxVec3(0,1,0);

		//table top
		PxActor* staticBox = pScene->createActor(actorDesc);
		staticBox->raiseBodyFlag(NX_BF_FROZEN);
		*/

		//2) Create cube
		//PxReal density = 1.0f;
		PxTransform transform(PxVec3(0.0f, 1.0f, 0.0f), PxQuat::createIdentity());
		PxVec3 dimensions(1.0f,0.1f,1.0f);
		PxBoxGeometry geometry(dimensions);

		PxRigidStatic *actor = PxCreateStatic(*pPhysicsSDK, transform, geometry, *pMaterial);


		if (!actor)
			cerr<<"create actor failed!"<<endl;

		pScene->addActor(*actor);
	}

	static PxPhysics* pPhysicsSDK;
	static PxCooking* pCooking;
	static PxScene* pScene;
	static PxReal timeStep;
	static PxFoundation* foundation;

	PxCloth* pCloth;

	PxMaterial* pMaterial;

public:
	std::vector<PxVec3> pos;
	std::vector<PxVec3> normal;
	std::vector<glm::vec2> uvs;
	std::vector<PxU32> indices;

	PxU32 numIndices, numVertices;
};

PxPhysics*		PhysXSimulator::pPhysicsSDK		= NULL;
PxCooking*		PhysXSimulator::pCooking		= NULL;
PxScene*		PhysXSimulator::pScene			= NULL;
PxFoundation*	PhysXSimulator::foundation		= NULL;
PxReal			PhysXSimulator::timeStep		= 1.0f/60;
#endif
