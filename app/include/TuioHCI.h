#ifndef TUIOHCI_H_
#define TUIOHCI_H_

#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "AbstractHCI.h"
#include "MVRCore/AbstractCamera.H"
#include <MVRCore/CameraOffAxis.H>
#include "CFrameMgr.H"
#include "app/include/GPUMesh.h"
#include "app/include/GLSLProgram.h"


class TuioHCI : public AbstractHCI {

public:
	TuioHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr);
	virtual ~TuioHCI();
	void update(const std::vector<MinVR::EventRef> &events) ;
    void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);
	void initializeContextSpecificVars(int threadId,MinVR::WindowRef window);
	void TuioHCI::initVBO(int threadId);

private:
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	std::map<int, MinVR::EventRef> registeredEvents; 
	std::shared_ptr<GPUMesh> cubeMesh;
	std::shared_ptr<GLSLProgram> shader;
	std::map<int, GLuint> _vboId;
	std::shared_ptr<CFrameMgr> cFrameMgr;


};

#endif /* TUIOHCI_H_ */