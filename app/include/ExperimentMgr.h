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
#include "app/include/NewAnchoredExperimentHCI.h"
#include <MVRCore/ConfigVal.H>
#include <MVRCore/Time.h>
#include "app/include/LikertHCI.h"
#include "OrigAnchoredHCI.h"
#include "OrigXZRotExperimentHCI.h"
#include "OrigYTransExperimentHCI.h"

typedef std::shared_ptr<class ExperimentMgr> ExperimentMgrRef;
static const double nearEnough = 0.03;

class ExperimentMgr {
public:
	ExperimentMgr(CurrentHCIMgrRef currentHCIMgr, CFrameMgrRef cFrameMgr, MinVR::AbstractCameraRef camera, TextureMgrRef texMan, FeedbackRef feedback); //maybe need ampersand
	virtual ~ExperimentMgr();
	void advance (bool newOld);
	void initializeContextSpecificVars(int threadId, MinVR::WindowRef window);
	bool checkFinish();
	void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	void resetTimer();
	
	MinVR::TimeStamp trialStart;
	MinVR::TimeStamp trialEnd;
	int HCIExperiment;

private:
	void initGL();
	CurrentHCIMgrRef currentHCIMgr;
	TetrahedronRef tetra;
	int transformIndex;
	std::vector<glm::dmat4> transforms;
	TextureMgrRef texMan;
	FeedbackRef feedback;
	int trialCount; // 0 to 4
	bool newAnchored;
	bool inPosition;
	bool secondTimer;
	bool showCompleteTrial;
	int trialSet;
	MinVR::TimeStamp startInZone;
	double totalTimeInZone;
	MinVR::TimeStamp startTime;
	MinVR::TimeStamp t2;
	glm::dmat4 transform;
	CFrameMgrRef cFrameMgr;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	std::vector<glm::dmat4> transMats;
	std::vector<glm::dmat4> rotMats;
	std::vector<glm::dmat4> combinedMats;
	int likertCount;
	int numTrials;
	int numPracticeTrials;
	
};

#endif /* EXPERIMENTMGR_H_ */
