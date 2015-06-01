#include "app/include/TuioHCI.h"
#include <boost/algorithm/string/predicate.hpp>
#include "app/include/GLSLProgram.h"
#include "app/include/GPUMesh.h"


TuioHCI::TuioHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan) : AbstractHCI(cFrameMgr){
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;
	
}

TuioHCI::~TuioHCI(){

}

void TuioHCI::initializeContextSpecificVars(int threadId,
		MinVR::WindowRef window) {
	
	initVBO(threadId);
	std::cout<<"TuioHCIinit is been called"<<std::endl;

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}
}

void TuioHCI::initVBO(int threadId) {
	GLfloat vertices[]  = { 0.25f, 0.0f, 0.25f,  -0.25f, 0.0f, -0.25f,  -0.25f, 0.0f, 0.25f};
	GLfloat normals[]   = { 0, 1, 0,   0, 1, 0,   0, 1, 0};
	GLfloat texture[] = {1,1,0, 0,0,0, 0,1,0 };//thrid coordinate is never use. only have it so the for loop could work
	
	std::vector<int> cubeIndices;
	std::vector<GPUMesh::Vertex> cubeData;
	GPUMesh::Vertex vert;
	for(int i=0; i < 9; i = i +3){
		vert.position = glm::dvec3(vertices[i],vertices[i+1],vertices[i+2]);
		vert.normal = glm::normalize(glm::dvec3(normals[i],normals[i+1],normals[i+2]));
		vert.texCoord0 = glm::dvec2(texture[i],texture[i+1]);
		cubeData.push_back(vert);
		cubeIndices.push_back(cubeData.size()-1);

	}
	_vboId[threadId] = GLuint(0);

	cubeMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*cubeData.size(), sizeof(int)*cubeIndices.size(), 0, cubeData, sizeof(GPUMesh::Vertex)*cubeData.size(), &cubeIndices[0]));
	


	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initVBO: "<<err<<std::endl;
	}
}

void TuioHCI::initGL() {

	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tuioPhong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tuioPhong.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

}

// this function produces a map, which we can later query to draw things.
void TuioHCI::update(const std::vector<MinVR::EventRef> &events){

	for(int i=0; i < events.size(); i++) {
		std::string name = events[i]->getName();
		int id = events[i]->getId();

	

		if (boost::algorithm::starts_with(name, "TUIO_Cursor_up")) {
			// delete the cursor down associated with this up event

			// This is to update the map
			std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(id); 
			if (it != registeredTouchData.end()) { // if id is found
				registeredTouchData.erase(it);		//erase value associate with that it
				std::cout << "UP" <<std::endl;
			}

			
		} else if (boost::algorithm::starts_with(name, "TUIO_Cursor_down")) {
			// always add a new one on DOWN
			glm::dvec3 roomCoord = convertScreenToRoomCoordinates(events[i]->get2DData());
			TouchDataRef datum(new TouchData(events[i], roomCoord));

			registeredTouchData.insert(std::pair<int, TouchDataRef>(id, datum));
			//cubeMesh->updateVertexData(int startByteOffset, int vertexOffset, const std::vector<Vertex> &data);

			std::cout << "DOWN" <<std::endl;
			//glm::dvec2 data = events[i]->get2DData();
			//std::cout << data.x << ", " << data.y /*<< ", " << data.z << ", " << data.w  */<< std::endl;

		} else if (boost::algorithm::starts_with(name, "TUIO_CursorMove")) {
			// update the map with the move event
			// if the corresponding id was down, make it a move event

			std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(id); 

			if (it != registeredTouchData.end()) { // if id is found
				glm::dvec3 roomCoord = convertScreenToRoomCoordinates(glm::dvec2(events[i]->get4DData()));
				it->second->setCurrentEvent(events[i]);
				it->second->setCurrRoomPos(roomCoord);

				
				std::cout << "MOVE ";

				if (registeredTouchData.size() == 1) {//only one finger on screen
					glm::dmat4 transMat(glm::translate(glm::dmat4(1.0f), -1.0*it->second->roomPositionDifference()));
					// negate the translation so this is actually virtual to room space
					glm::dmat4 newTransform = cFrameMgr->getRoomToVirtualSpaceFrame()*transMat;
					cFrameMgr->setRoomToVirtualSpaceFrame(newTransform);
					
				} else if (registeredTouchData.size() == 2) {
					
					// translate to origin, the other touch point
					glm::dvec3 centOfRot;
					std::map<int, TouchDataRef>::iterator iter;

					for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) { // finding other touch point.
						if (id != iter->second->getCurrentEvent()->getId()) { // found other touch point
							centOfRot = iter->second->getCurrRoomPos();
							std::cout<<"center of rotation found"<<std::endl;
							std::cout<<"it id"<<id<<std::endl;
							std::cout<<"iter id"<<iter->second->getCurrentEvent()->getId()<<std::endl;
							break;
						}
					}
					glm::dmat4 transMat(glm::translate(glm::dmat4(1.0), -1.0*centOfRot));

					// rotate
					glm::dvec3 prevDiffBetweenTwoPoints = glm::normalize(it->second->getPrevRoomPos() - centOfRot);
					glm::dvec3 currDiffBetweenTwoPoints = glm::normalize(roomCoord - centOfRot);
					glm::dvec3 crossProd = glm::cross(prevDiffBetweenTwoPoints,currDiffBetweenTwoPoints);
					double theta = glm::dot(prevDiffBetweenTwoPoints,currDiffBetweenTwoPoints);
					if(crossProd.y < 0){
						theta = -theta;
					}
					glm::dmat4 rotMat = glm::rotate(glm::dmat4(1.0) , -theta, glm::dvec3(0,1,0));
					glm::dmat4 transBack(glm::translate(glm::dmat4(1.0), centOfRot));
					
					std::cout<<"rotMat"<<glm::to_string(rotMat)<<std::endl;
					

					// scale
					
					//glm::dmat4 scaleMat = glm::scale(
						//glm::dmat4(1.0f),
						//scaleBy); 

					//glm::dmat4 rotScale;
					glm::dmat4 newTransform = cFrameMgr->getRoomToVirtualSpaceFrame() * transBack * rotMat * transMat;
					cFrameMgr->setRoomToVirtualSpaceFrame(newTransform);
					
				}


			}



			// update the cframe which will rotate the cube from App
			// cframe is pass by reference, so just mutate it here.

			//single finger touch point: then do translation
			


			
		} 
	}



}



void TuioHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window){


	/*glm::dmat4 translate = glm::translate(glm::dmat4(1.0f), glm::dvec3(0.0f, 0.0f, -5.0f));
	camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), glm::dvec3(0.0f, -3.5f, -3.0f)));
	glm::dvec2 rotAngles(-20.0, 45.0);
	glm::dmat4 rotate1 = glm::rotate(translate, rotAngles.y, glm::dvec3(0.0,1.0,0.0));
	camera->setObjectToWorldMatrix(glm::rotate(rotate1, rotAngles.x, glm::dvec3(1.0,0,0)));*/

	shader->use();
	MinVR::CameraOffAxis* offAxisCam = dynamic_cast<MinVR::CameraOffAxis*>(camera.get());

	shader->setUniform("projection_mat", offAxisCam->getLastAppliedProjectionMatrix());
	shader->setUniform("view_mat", offAxisCam->getLastAppliedViewMatrix());
	
	//shader->setUniform("normal_matrix", glm::dmat3(offAxisCam->getLastAppliedModelMatrix()));
	//glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCam->getLastAppliedViewMatrix()), 3));
	//shader->setUniform("eye_world", eye_world);
	texMan->getTexture(threadId, "Koala2")->bind(0);
	shader->setUniform("koalaTextureSampler",0);


	std::map<int, TouchDataRef>::iterator it;
	glm::dvec3 roomCoord;

	glBindVertexArray(cubeMesh->getVAOID());

	for(it = registeredTouchData.begin(); it != registeredTouchData.end(); ++it) {
		
		TouchDataRef event = it->second;
		
		roomCoord = event->getCurrRoomPos();
		
		// new matrix for each triangle
		camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), roomCoord));
		shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
		// draw triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
	}
}

glm::dvec3 TuioHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}