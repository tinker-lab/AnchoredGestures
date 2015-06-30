#ifndef EXPERIMENTMGR_H_
#define EXPERIMENTMGR_H_

// what things do we need to include anyway?
#include "app/include/Tetrahedron.h"
#include "app/include/AbstractHCI.h"
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
#include "CFrameMgr.H"
#include "MVRCore/AbstractCamera.H"
#include "app/include/TextureMgr.h"
#include "app/include/TestHCI.h"
#include "app/include/Feedback.h"
#include "app/include/NewYTransExperimentHCI.h"
#include "app/include/CurrentHCIMgr.h"
#include "app/include/NewXZRotExperimentHCI.h"

typedef std::shared_ptr<class ExperimentMgr> ExperimentMgrRef;

class ExperimentMgr {
public:
	ExperimentMgr(CurrentHCIMgrRef currentHCIMgr, CFrameMgrRef cFrameMgr, MinVR::AbstractCameraRef camera, TextureMgrRef texMan, FeedbackRef feedback); //maybe need ampersand
	virtual ~ExperimentMgr();
	void switchHCI ();
	void switchTrial ();
	void advance ();
	void initializeContextSpecificVars(int threadId, MinVR::WindowRef window);
	glm::dmat4 getTransforms();

	void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);


private:
	void initGL();
	CurrentHCIMgrRef currentHCIMgr;
	TetrahedronRef tetra;
	int transformIndex;
	std::vector<glm::dmat4> transforms;
	TextureMgrRef texMan;
	FeedbackRef feedback;
	int experimentNumber;
	int trialNumber; // 0 to 4
	bool newAnchored;
	CFrameMgrRef cFrameMgr;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	
};

#endif /* EXPERIMENTMGR_H_ */
