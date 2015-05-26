#ifndef TUIOHCI_H_
#define TUIOHCI_H_

#include "AbstractHCI.h"
#include "MVRCore/AbstractCamera.H"
#include <MVRCore/CameraOffAxis.H>
#include "CFrameMgr.H"


class TuioHCI : public AbstractHCI {

public:
	TuioHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr);
	virtual ~TuioHCI();
	void update(const std::vector<MinVR::EventRef> &events) ;
    void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);
private:
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;


};

#endif /* TUIOHCI_H_ */