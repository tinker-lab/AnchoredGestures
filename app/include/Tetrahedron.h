#ifndef TETRAHEDRON_H
#define TETRAHEDRON_H

#include "app/include/GPUMesh.h"
#include <GLFW/glfw3.h>
#include "MVRCore/Event.H"
#include <glm/glm.hpp>
#include <memory>
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include "CFrameMgr.H"
#include "app/include/GLSLProgram.h"
#include <MVRCore/CameraOffAxis.H>
#include <MVRCore/DataFileUtils.H>
#include <glm/gtc/matrix_transform.hpp>
#include "app/include/TextureMgr.h"
#include "MVRCore/AbstractCamera.H"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef std::shared_ptr<class Tetrahedron> TetrahedronRef;

class Tetrahedron {
public:
	Tetrahedron(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan);
	~Tetrahedron();
	void initializeContextSpecificVars(int threadId);
	void initVBO(int threadId);
	void initGL() ;

	glm::dvec3 Tetrahedron::getPosition(double latitude, double longitude); 

	void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window, std::string textureName);
	void makeCylinder(glm::dvec3 pointA, glm::dvec3 pointB);
	void makeSphere(glm::dvec3 center);

	
private:
	std::map<int, GLuint> _vboId;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;

	GPUMeshRef cylinderMesh; // holds six cylinders
	GPUMeshRef sphereMesh; //only one sphere
	int GPUcylinderOffset;
	TextureMgrRef texMan; 
	CFrameMgrRef cFrameMgr;
	std::shared_ptr<GLSLProgram> tetraShader;
	std::vector<int> cylinderIndices;
	std::vector<GPUMesh::Vertex> cylinderData;
	std::vector<GPUMesh::Vertex> sphereData;
	std::vector<int> sphereIndices;

};

#endif /* TETRAHEDRON_H_ */
