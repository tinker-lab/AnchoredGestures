#include "app/include/TuioHCI.h"
#include <boost/algorithm/string/predicate.hpp>
#include "app/include/GLSLProgram.h"
#include "app/include/GPUMesh.h"

TuioHCI::TuioHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr) : AbstractHCI(cFrameMgr){
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
}

TuioHCI::~TuioHCI(){

}

void TuioHCI::initializeContextSpecificVars(int threadId,
		MinVR::WindowRef window) {
	
	initVBO(threadId);
	

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}

	cFrameMgr.reset(new CFrameMgr());
	
}

// this function produces a map, which we can later query to draw things.
void TuioHCI::update(const std::vector<MinVR::EventRef> &events){

	// touch down, move, up
	for(int i=0; i < events.size(); i++) {
		std::string name = events[i]->getName();
		int id = events[i]->getId();

	

		if (boost::algorithm::starts_with(name, "TUIO_Cursor_up")) {
			// delete the cursor down associated with this up event

			// This is to update the map
			std::map<int, MinVR::EventRef>::iterator it = registeredEvents.find(id); 
			if (it != registeredEvents.end()) { // if id is found
				registeredEvents.erase(it);		//erase value associate with that it
				std::cout << "UP" <<std::endl;
			}

			
		} else if (boost::algorithm::starts_with(name, "TUIO_Cursor_down")) {
			// always add a new one on DOWN

			registeredEvents.insert(std::pair<int, MinVR::EventRef>(id, events[i]));

			std::cout << "DOWN" <<std::endl;
		} else if (boost::algorithm::starts_with(name, "TUIO_CursorMove")) {
			// update the map with the move event
			// if the corresponding id was down, make it a move event


			std::map<int, MinVR::EventRef>::iterator it = registeredEvents.find(id); 
			if (it != registeredEvents.end()) { // if id is found
				it->second = events[i];			// update the map
				std::cout << "MOVE" <<std::endl;
			}


			
		} 
	}


	

	// store current position in a map, id is keys (touch objects if class for touch exists)

	// gesture recognition, which uses the map produced above, and maybe some addtl. data
	// this chooses what kind of matrix transforms we use

	// current position will need a mat transform, virtual to room

	// new class called touch?

	// show content:
  /*for (std::map<int,MinVR::EventRef>::iterator it = registeredEvents.begin(); it != registeredEvents.end(); ++it) {
    std::cout << it->first << " => " << it->second->toString() << '\n';
  }*/
	
}

void TuioHCI::initVBO(int threadId)
{
	GLfloat vertices[]  = { 1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f,-1.0f, 1.0f};
	GLfloat normals[]   = { 0, 0, 1,   0, 0, 1,   0, 0, 1};
	std::vector<int> cubeIndices;
	std::vector<GPUMesh::Vertex> cubeData;
	GPUMesh::Vertex vert;
	for(int i=0; i < 9; i = i +3){
		vert.position = glm::dvec3(vertices[i],vertices[i+1],vertices[i+2]);
		vert.normal = glm::normalize(glm::dvec3(normals[i],normals[i+1],normals[i+2]));
		//vert.texCoord0 = glm::dvec2(01,0.9);
		cubeData.push_back(vert);
		cubeIndices.push_back(cubeData.size()-1);

	}

	cubeMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*cubeData.size(), sizeof(int)*cubeIndices.size(),0,cubeData,sizeof(int)*cubeIndices.size(), &cubeIndices[0]));
	_vboId[threadId] = GLuint(0);


	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initVBO: "<<err<<std::endl;
	}
}
void TuioHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window){

	// want to draw for each event.
	//std::map<int,MinVR::EventRef>::iterator it;
	//for (it = registeredEvents.begin(); it != registeredEvents.end(); ++it) {
	//	//draw each events
	//}


	//camera->setObjectToWorldMatrix(virtualToRoomFrame * cframe);
	const int numIndices = (int)(cubeMesh->getFilledIndexByteSize()/sizeof(int));
	glBindVertexArray(cubeMesh->getVAOID());
	glDrawArrays(GL_TRIANGLES, 0, numIndices);

}

glm::dvec3 TuioHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}