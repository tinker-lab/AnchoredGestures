#include "app/include/TuioHCI.h"


TuioHCI::TuioHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr) : AbstractHCI(cFrameMgr){
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
}

TuioHCI::~TuioHCI(){

}

void TuioHCI::update(const std::vector<MinVR::EventRef> &events){


}

void TuioHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window){


}

glm::dvec3 TuioHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}