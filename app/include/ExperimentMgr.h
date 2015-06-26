#ifndef EXPERIMENTMGR_H_
#define EXPERIMENTMGR_H_

// what things do we need to include anyway?
#include "app/includes/Tetrahedron.h"
#include "app/includes/AbstractHCI.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include "CFrameMgr.H"
#include "app/include/GLSLProgram.h"
#include <MVRCore/CameraOffAxis.H>
#include <MVRCore/DataFileUtils.H>
#include <glm/gtc/matrix_transform.hpp>
#include "app/include/TextureMgr.h"
#include "MVRCore/AbstractCamera.H"

typedef std::shared_ptr<class TextureMgr> ExperimentMgrRef;

class ExperimentMgr {
public:
	ExperimentMgr(AbstractHCIRef currentHCI); //maybe need ampersand
	virtual ~ExperimentMgr();

private:
	void initGL();
	void initVBO(int threadId, MinVR::WindowRef window);
	AbstractHCIRef currentHCI;
	TetrahedronRef tetra;
	int transformIndex;
	std::vector<glm::dmat4> transforms;
	
	int experimentNumber;
	int trialNumber; // 0 to 4
	bool newAnchored;  
	
};

#endif /* EXPERIMENTMGR_H_ */
