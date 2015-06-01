#include "app/include/TouchData.h"

TouchData::TouchData(MinVR::EventRef event, glm::dvec3 currRoomPos) {
	prevEvent = event; // should I do this?
	currEvent = event;

	this->currRoomPos = currRoomPos;
	prevRoomPos = currRoomPos;
	
}

TouchData::~TouchData() {

}

void TouchData::setCurrentEvent(MinVR::EventRef event) {
	prevEvent = currEvent;
	currEvent = event;
}

MinVR::EventRef TouchData::getCurrentEvent() {
	return currEvent;
}

MinVR::EventRef TouchData::getPreviousEvent() {
	return prevEvent;
}

void TouchData::setCurrRoomPos(glm::dvec3 pos) {
	prevRoomPos = currRoomPos;
	currRoomPos = pos;
}

glm::dvec3 TouchData::getCurrRoomPos() {
	return currRoomPos;
}


glm::dvec3 TouchData::getPrevRoomPos() {
	return prevRoomPos;
}

//  gives you the distance in screen coordinates
glm::dvec3 TouchData::roomPositionDifference() {
	return currRoomPos - prevRoomPos;
}