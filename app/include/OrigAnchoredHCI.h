
#ifndef ORIGANCHOREDHCI_H
#define ORIGANCHOREDHCI_H
/*
#include "AbstractHCI.h"
#include "MVRCore/AbstractCamera.H"
#include <MVRCore/CameraOffAxis.H>
#include "CFrameMgr.H"
#include "app/include/GPUMesh.h"
#include "app/include/GLSLProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <MVRCore/DataFileUtils.H>
#include "app/include/TextureMgr.h"
#include "app/include/TouchData.h"
#include <glm/gtx/string_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>"
#include <MVRCore/Time.h>
#include <iterator>

class OrigAnchoredHCI : public AbstractHCI {
public:
	OrigAnchoredHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef textMan);
	virtual ~OrigAnchoredHCI();
	void update(const std::vector<MinVR::EventRef> &events);
    void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);

private:
	bool offerTouchDown(MinVR::EventRef event);
	bool offerTouchUp(int id);
	bool offerTouchMove(MinVR::EventRef event);
	void updateTrackers(const glm::dmat4 &rightTrackerFrame, const glm::dmat4 &leftTrackerFrame);
	void resetTouchs();
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);
	void determineTouchToHandCoorespondence(TouchDataRef touch);

	TextureMgrRef texMan;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	glm::dvec3 _centerAxis;
	bool _touch1IsValid;
	bool _touch2IsValid;
	TouchDataRef _touch1;
	TouchDataRef _touch2;
	glm::dvec3 _lastRotationAxis;
	glm::dvec3 _lastLeftAxis;
	glm::dvec3 _lastRightAxis;
	std::vector<glm::dvec3> _leftAxisHistory;
	std::vector<glm::dvec3> _rightAxisHistory;
	std::vector<float> _leftAngleHistory;
	std::vector<float> _rightAngleHistory;
	bool _rotating;
	bool _translating;
	glm::dmat4 _previousRightTrackerFrame;
	glm::dmat4 _previousLeftTrackerFrame;
	glm::dmat4 _currentRightTrackerFrame;
	glm::dmat4 _currentLeftTrackerFrame;
};
*/
#endif
