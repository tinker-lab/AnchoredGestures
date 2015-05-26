#include "app/include/TuioHCI.h"
#include <boost/algorithm/string/predicate.hpp>

TuioHCI::TuioHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr) : AbstractHCI(cFrameMgr){
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
}

TuioHCI::~TuioHCI(){

}

// this function produces a map, which we can later query to draw things.
void TuioHCI::update(const std::vector<MinVR::EventRef> &events){


	// touch down, move, up
	for(int i=0; i < events.size(); i++) {
		std::string name = events[i]->getName();
		int id = events[i]->getId();
		

		if (boost::algorithm::starts_with(name, "TUIO_Cursor_up")) {
			// delete the cursor down associated with this up event
			// add the up event

			registeredEvents.insert(std::pair<int,MinVR::EventRef>(id, events[i]));

			std::cout << "UP" <<std::endl;
		} else if (boost::algorithm::starts_with(name, "TUIO_Cursor_down")) {
			// always add a new one on DOWN


			//registeredEvents.insert(0, events[i]); 
			std::cout << "DOWN" <<std::endl;
		} else if (boost::algorithm::starts_with(name, "TUIO_CursorMove")) {
			// update the map with the move event
			// if the corresponding id was down, make it a move event


			std::cout << "MOVE" <<std::endl;
		} 
	}


	;

	// store current position in a map, id is keys (touch objects if class for touch exists)

	// gesture recognition, which uses the map produced above, and maybe some addtl. data
	// this chooses what kind of matrix transforms we use

	// current position will need a mat transform, virtual to room

	// new class called touch?

	// show content:
  for (std::map<int,MinVR::EventRef>::iterator it = registeredEvents.begin(); it != registeredEvents.end(); ++it) {
    std::cout << it->first << " => " << it->second->toString() << '\n';
  }
	
}

void TuioHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window){


}

glm::dvec3 TuioHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}