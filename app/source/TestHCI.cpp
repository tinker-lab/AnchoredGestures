#include "app/include/TestHCI.h"

const double THRESH = 0.00134;


TestHCI::TestHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan) : AbstractHCI(cFrameMgr){
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;
	startTime = getCurrentTime();
	
}

TestHCI::~TestHCI(){

}

void TestHCI::initializeContextSpecificVars(int threadId,MinVR::WindowRef window) {

	initVBO(threadId);
	std::cout<<"TuioHCIinit is been called"<<std::endl;
	prevHandPos1 = glm::dvec3(0.0, -1.0, 0.0);
	prevHandPos2 = glm::dvec3(0.0, -1.0, 0.0);
	xzRotFlag = false;

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<< err << std::endl;
	}
}

void TestHCI::initVBO(int threadId) {
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

void TestHCI::initGL() {

	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tuioPhong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tuioPhong.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

}

//void TestHCI::testForCrazyInput

// this function produces a map, which we can later query to draw things.
void TestHCI::update(const std::vector<MinVR::EventRef> &events){

	//later to be pass into closestTouchPair function
	glm::dvec3 pos1;
	glm::dvec3 pos2;	
	double minDistance = DBL_MAX; 
	MinVR::TimeStamp timestamp;

	//boolean flags
	bool xzTrans = false;
	glm::dmat4 xzTransMat = dmat4(0.0);
	bool yTrans = false;
	glm::dmat4 yTransMat = dmat4(0.0);


	// for loop should set flags and do updates on data structures
	// after for loop we look at set flags and apply the correct transforms?
	for(int i=0; i < events.size(); i++) {

		timestamp = events[i]->getTimestamp();
		std::string name = events[i]->getName();
		int id = events[i]->getId();

		if (boost::algorithm::starts_with(name, "TUIO_Cursor_up")) {
			// delete the cursor down associated with this up event
			std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(id); 

			if (it != registeredTouchData.end()) { // if id is found
				registeredTouchData.erase(it);	   //erase value associate with that it
				//std::cout << "UP" <<std::endl;
			}

		} else if (boost::algorithm::starts_with(name, "TUIO_Cursor_down")) {
			// always add a new one on DOWN
			glm::dvec3 roomCoord = convertScreenToRoomCoordinates(events[i]->get2DData());
			TouchDataRef datum(new TouchData(events[i], roomCoord));
			registeredTouchData.insert(std::pair<int, TouchDataRef>(id, datum));
			//std::cout << "DOWN " << events[i]->getId() <<std::endl;

		} else if (boost::algorithm::starts_with(name, "TUIO_CursorMove")) {
			// update the map with the move event
			// if the corresponding id was down, make it a move event
			std::map<int, TouchDataRef>::iterator it = registeredTouchData.find(id); 
			//std::cout << "Move " << events[i]->getId() <<std::endl;

			if (it != registeredTouchData.end()) { // if id is found
				glm::dvec2 screenCoord (events[i]->get4DData());
				glm::dvec3 roomCoord = convertScreenToRoomCoordinates(glm::dvec2(events[i]->get4DData()));

				// update map
				it->second->setCurrentEvent(events[i]);
				it->second->setCurrRoomPos(roomCoord);

				// translate 
				if (registeredTouchData.size() == 1 && !xzRotFlag) {  //only one finger on screen
					xzTrans = true;
					// negate the translation so this is actually virtual to room space
					xzTransMat = glm::translate(glm::dmat4(1.0f), -1.0*it->second->roomPositionDifference());
					
					
				} else if (registeredTouchData.size() == 2 && !xzRotFlag) {
					
					// translate to origin using coord of the other touch point
					glm::dvec3 centOfRot;
					std::map<int, TouchDataRef>::iterator iter;

					for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) { // finding other touch point.
						if (id != iter->second->getCurrentEvent()->getId()) { // found other touch point
							centOfRot = iter->second->getCurrRoomPos();
							break;
						}
					}

					// translate to origin
					glm::dmat4 transMat(glm::translate(glm::dmat4(1.0), -1.0*centOfRot));


				
					glm::dmat4 rotMat = glm::dmat4(1.0);
					glm::dmat4 scaleMat = glm::dmat4(1.0);
					if(glm::abs(glm::length(it->second->getPrevRoomPos()) - glm::length(it->second->getCurrRoomPos())) > THRESH) {

						std::cout<<"using the filtered pos in rotate and scale"<<std::endl;
						// rotate

						//// 0 vector guard
						glm::dvec3 prevDiffBetweenTwoPoints;
						if (glm::length(roomCoord - centOfRot) > 0.0) {
							prevDiffBetweenTwoPoints = glm::normalize(it->second->getPrevRoomPos() - centOfRot);
						} 
							
						//// 0 vector guard
						glm::dvec3 currDiffBetweenTwoPoints;
						if (glm::length(roomCoord - centOfRot) > 0.0) {
							currDiffBetweenTwoPoints = glm::normalize(roomCoord - centOfRot);
						} 
							
						
						
						// both distances are normalized
						glm::dvec3 crossProd = glm::cross(prevDiffBetweenTwoPoints,currDiffBetweenTwoPoints);
						double theta = glm::acos(glm::dot(prevDiffBetweenTwoPoints,currDiffBetweenTwoPoints));
						if(crossProd.y < 0){
							theta = -theta;
						}

						//std::cout << "Rotation Angle Theta: " << theta << std::endl;
						// glm::rotate takes degrees! Madness.
						rotMat = glm::rotate(glm::dmat4(1.0) , glm::degrees(-theta), glm::dvec3(0.0, 1.0, 0.0));

						// scale
						double prevDistanceDiff = glm::length(it->second->getPrevRoomPos() - centOfRot);
						double currDistanceDiff = glm::length(roomCoord - centOfRot);

						//std::cout << prevDistanceDiff/currDistanceDiff << std::endl;

						// might move this into a more general function
						// to test for crazy input
						/*if (glm::dvec3(prevDistanceDiff/currDistanceDiff)) {
						
						}*/
						glm::dvec3 scaleBy = glm::dvec3(prevDistanceDiff/currDistanceDiff);
						scaleMat = glm::scale(
							glm::dmat4(1.0),
							scaleBy); 

					}

					
					// translate back
					glm::dmat4 transBack(glm::translate(glm::dmat4(1.0), centOfRot));

					// combine transforms
					glm::dmat4 newTransform = cFrameMgr->getRoomToVirtualSpaceFrame() * transBack * scaleMat *rotMat * transMat;
					cFrameMgr->setRoomToVirtualSpaceFrame(newTransform);
					
				} 
			}
		} // end of TUIO if/else-if block

		// find closest pair of TouchPoints
		if (registeredTouchData.size() > 1) {
			closestTouchPair(registeredTouchData , pos1, pos2, minDistance);
		}

		// Update hand positions
		if (name == "Hand_Tracker1") {
			//std::cout << "Inside hand tracking event " << std::endl;
			//only enter one time to init prevHandPos1
			if (prevHandPos1.y == -1.0) {
				glm::dvec3 currHandPos1 (events[i]->getCoordinateFrameData()[3]);
				prevHandPos1 = currHandPos1;
				initRoomPos = true;
			} else {
				prevHandPos1 = currHandPos1;
				currHandPos1 = glm::dvec3(events[i]->getCoordinateFrameData()[3]);
			} 
		} //HandTracker1 if block ends here

		if(name == "Hand_Tracker2"){
			if (prevHandPos2.y == -1.0) {
				glm::dvec3 currHandPos2 (events[i]->getCoordinateFrameData()[3]);
				prevHandPos2 = currHandPos2;
			} else {
				prevHandPos2 = currHandPos2;
				currHandPos2 = glm::dvec3(events[i]->getCoordinateFrameData()[3]);
			} 
		}

		
	} // end of data update for loop

	///// Apply the correct matrix transforms based on updated state (booleans, registeredTouchData, instance variables)

	if (xzTrans) {
		Translate(xzTransMat);
	} else if (yTrans) {
		Translate(yTransMat);
	}

	if (minDistance < 0.025 && currHandPos1 != prevHandPos1) {

		xzRotFlag = true;
		std::cout << "Inside XZRot Mode" << std::endl;

	}

	if (xzRotFlag) { // might have to be xzRotFlag and not any other flag 


		if(initRoomPos){

			initRoomTouchCentre = 0.5*(pos1 + pos2);
			initRoomPos = false;

		}

		glm::dvec3 centOfRot (0.0);
		glm::dvec3 prevHandToTouch;

		//calculate the current handToTouch vector
		glm::dvec3 roomTouchCentre = 0.5*(pos1 + pos2);
		glm::dvec3 currHandToTouch = roomTouchCentre - currHandPos1;
		prevHandToTouch = roomTouchCentre - prevHandPos1;

		//set up the 2 vertices for a squre boundry for the gesture
		glm::dvec3 upRight = glm::dvec3(initRoomTouchCentre.x+0.07, 0.0, initRoomTouchCentre.z+0.07);
		glm::dvec3 lowLeft = glm::dvec3(initRoomTouchCentre.x-0.07, 0.0, initRoomTouchCentre.z-0.07);


		// this if-else block for setting xzRotFlag,
		// also for grabbing xzCentOfRot
		if(registeredTouchData.size() == 0) { //if no touch on screen then automatically exit the xzrot mode
			xzRotFlag = false;
			initRoomPos = true;
			std::cout<<"no touchyyy so I quit"<<std::endl;
		}
		else { //if there are touch(s) then check if the touch is in bound of the rectangle

			bool setxzRotFlag = true;
			std::map<int, TouchDataRef>::iterator iter;
			for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) {


				// not exactly sure why roomPos. > upRight.z , I think it should be <. but that doesn't work
				if(!(iter->second->getCurrRoomPos().x > upRight.x || iter->second->getCurrRoomPos().z > upRight.z ||iter->second->getCurrRoomPos().x < lowLeft.x ||iter->second->getCurrRoomPos().z < lowLeft.z)){ //you are in the box

					std::cout << "fingers in bound so STILL IN XZRot Mode" << std::endl;
					setxzRotFlag = false; 

				} else { // touch point not in box, assume as center of rotation
					centOfRot = iter->second->getCurrRoomPos();
					std::cout << "Cent of Rot set" << std::endl;
				}

				// only tries to change the xzRotFlag at the end of the data in the map
				if(iter == std::prev(registeredTouchData.end(),1) && setxzRotFlag) {
					xzRotFlag = false;
					initRoomPos = true;
					std::cout << "all fingers went out of bound so Out of XZRot Mode" << std::endl;
				}
			} // end for loop over registeredTouchData

		} // end if/else block





		double alpha = glm::acos(glm::dot(currHandToTouch,prevHandToTouch)); // angle between both vectors

		// get cross prod
		glm::dvec3 cross = glm::cross(glm::normalize(currHandToTouch), glm::normalize(prevHandToTouch)); 
		glm::dvec3 normProjCross = glm::normalize(glm::dvec3(cross.x, 0.0, cross.z));


		//std::cout << "Cross: " << glm::to_string(cross) << std::endl;

		// project cross prod onto the screen, get a length
		double lengthOfProjection = glm::dot(cross, normProjCross); 

		// projected cross prod 
		glm::dvec3 projectedCrossProd = lengthOfProjection * normProjCross; 

		// modified angle that we rotate with
		// 73.5 to make it more sensitive
		alpha = alpha * lengthOfProjection;

		// make a matrix transform, one for x rotation, one for z
		// we're rotating around projectedCrossProd

		glm::dmat4 XZRotMat = glm::rotate(glm::dmat4(1.0), glm::degrees(alpha), normProjCross);

		// have translations when we have a touch point not in the bounding box

		// translate to origin
		glm::dmat4 transMat(glm::translate(glm::dmat4(1.0), -1.0*centOfRot));
		// translate back
		glm::dmat4 transBack(glm::translate(glm::dmat4(1.0), centOfRot));	

		// put it into the matrix stack			
		cFrameMgr->setRoomToVirtualSpaceFrame(cFrameMgr->getRoomToVirtualSpaceFrame() * transBack * XZRotMat * transMat);

		//glBegin(GL_LINES);
		//	glVertex3f(roomTouchCentre.x, roomTouchCentre.y, roomTouchCentre.z);
		//	glVertex3f(roomTouchCentre.x + projectedCrossProd.x, roomTouchCentre.y + projectedCrossProd.y, roomTouchCentre.z + projectedCrossProd.z);
		//glEnd();




		// GL_STREAM_DRAW

	} // end xzRot Gesture

	// I think this is what this should look like...
	// Y translation gesture
	// if (YTranslate) {
	//		YTranslate()
	// }
	double prevHandsDist = glm::length(prevHandPos1 - prevHandPos2);
	double currHandsDist = glm::length(currHandPos1 - currHandPos2);
	std::cout << "curr - prev hand dist: " << glm::abs(currHandsDist - prevHandsDist) << std::endl;
	
	// weighted balloon gesture for Y translation
	if (registeredTouchData.size() > 3 && glm::abs(currHandsDist - prevHandsDist) < 0.045 && glm::abs(currHandsDist - prevHandsDist) > 0.0005) {
		
		//std::cout << "4 Touches" << std::endl;
		
		// to enumerate number of touches for each hand
		numTouchForHand1 = 0;
		numTouchForHand2 = 0;

		std::map<int, TouchDataRef>::iterator iter;
		for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) {
			
			glm::dvec3 currRoomPos (iter->second->getCurrRoomPos());
			bool belongsToHand1 = (glm::length(currHandPos1 - currRoomPos) <  glm::length(currHandPos2 - currRoomPos));
			if (belongsToHand1) {
				numTouchForHand1++;
			} else { // belongs to hand 2
				numTouchForHand2++;
			} 
		} // end touch enumeration

		/*std::cout << "Hand 1: " << numTouchForHand1 << std::endl;
		std::cout << "Hand 2: " << numTouchForHand2 << std::endl;*/

		// check if we have two points for each hand
		if (numTouchForHand1 == 2 && numTouchForHand2 == 2) {
			std::cout << "In Y Trans Mode" << std::endl;
			//calculate translate distance
			double prevHandsDist = glm::length(prevHandPos1 - prevHandPos2);
			double currHandsDist = glm::length(currHandPos1 - currHandPos2);

			double transBy = currHandsDist - prevHandsDist;
			glm::dvec3 yTransBy (0.0, transBy, 0.0);
			glm::dmat4 yTransMat (glm::translate(glm::dmat4(1.0), -yTransBy));

			cFrameMgr->setRoomToVirtualSpaceFrame(cFrameMgr->getRoomToVirtualSpaceFrame() * yTransMat);
		}

	}
	// this is bret's commented out line
	//updateHandPos(events);
}

void TestHCI::closestTouchPair(std::map<int, TouchDataRef> thisRegisteredTouchData, glm::dvec3 &pos1, glm::dvec3 &pos2, double &minDistance) {
	std::map<int, TouchDataRef>::iterator it1;
	std::map<int, TouchDataRef>::iterator it2;

	for(it1 = registeredTouchData.begin(); it1 != std::prev(registeredTouchData.end(), 1); it1++) {
		pos1 = it1->second->getCurrRoomPos();
		//std::cout << "pos1 : " << glm::to_string(pos1) << std::endl;

		for(it2 = std::next(it1, 1) ; it2 != registeredTouchData.end(); it2++){
			pos2 = it2->second->getCurrRoomPos();
			//std::cout << "pos2: " << glm::to_string(pos2) << std::endl;

			if(minDistance > glm::abs(glm::length(pos1-pos2))){
				minDistance = glm::abs(glm::length(pos1-pos2));
			}
		}
	}
	//std::cout << "pos1 : " << glm::to_string(pos1) << std::endl;
	//std::cout << "pos2: " << glm::to_string(pos2) << std::endl;
	//std::cout << "Calculating minDist: " << minDistance << std::endl;
}

void TestHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window){


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
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		
	}
}

glm::dvec3 TestHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}


void TestHCI::Translate(glm::dmat4 transMat){
	glm::dmat4 newTransform = cFrameMgr->getRoomToVirtualSpaceFrame()*transMat;
	cFrameMgr->setRoomToVirtualSpaceFrame(newTransform);
}

//void TestHCI::convertScreenToRoomCoordinates{}
//
//void TestHCI::convertScreenToRoomCoordinates{}


///// KINECT STUFF
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void TestHCI::updateHandPos(const std::vector<MinVR::EventRef>& events) {
//	//later to be pass into closestTouchPair function
//	glm::dvec3 pos1;
//	glm::dvec3 pos2;	
//	double minDistance = DBL_MAX;
//	glm::dvec3 initroomTouchCentre;
//
//	// find closest pair of TouchPoints
//	if (registeredTouchData.size() > 1) {
//		closestTouchPair(registeredTouchData , pos1, pos2, minDistance);
//
//			
//	}
//
//	// Get the current head position from motive
//	glm::dmat4 headTracker;
//	bool foundHead = false;
//	int i = (int)events.size()-1;
//	while ((i >= 0) && (events[i]->getName() != "Head_Tracker")) {
//		i--;
//	}
//	if (i >= 0) {
//		headTracker = events[i]->getCoordinateFrameData();
//		foundHead = true;
//	}
//	glm::dvec3 headPos(glm::column(headTracker, 3));
//
//	if (foundHead) {
//		// Find the kinect skeleton id who's head is closest to the motive head
//		std::string closest = "";
//		double closestDist = 99999999999;
//		for(int i=0; i < events.size(); i++) {
//			std::string name = events[i]->getName();
//			if (MinVR::startsWith(name, "SpineBase")) {
//				std::cout<<"FOUND A SKELETON"<<std::endl;
//				glm::dvec3 kinectHead = glm::dvec3(glm::column(events[i]->getCoordinateFrameData(),3));
//				double distance = glm::length(kinectHead - headPos);
//				if (distance < closestDist) {
//					closest = name.substr(9, 1);
//					closestDist = distance;
//				}
//			}
//		}
//		if (closest != "") {
//			// Now find the hand positions that coorespond with that skeleton
//			for(int i=0; i < events.size(); i++) {
//				std::string name = events[i]->getName();
//		
//				if (name == "HandRight"+closest) {
//					std::cout<<name<<std::endl;
//					if (prevHandPos1.y == -1.0){
//						glm::dvec3 currHandPos1 (events[i]->getCoordinateFrameData()[3]);
//						prevHandPos1 = currHandPos1;
//						initRoomPos = true;
//					} else {
//						prevHandPos1 = currHandPos1;
//						currHandPos1 = glm::dvec3(events[i]->getCoordinateFrameData()[3]);
//
//					} 
//
//					if (minDistance < 0.1 /*some arb value*/ && currHandPos1 != prevHandPos1) {
//				
//						xzRotFlag = true;
//						std::cout << "Inside XZRot Mode" << std::endl;
//			
//					}
//
//				
//					if (xzRotFlag) {
//
//			
//						if(initRoomPos){
//					
//							initRoomTouchCentre = 0.5*(pos1 + pos2);
//							initRoomPos = false;
//
//						}
//				
//				
//						glm::dvec3 prevHandToTouch;
//
//						//calculate the current handToTouch vector
//						glm::dvec3 roomTouchCentre = 0.5*(pos1 + pos2);
//						glm::dvec3 currHandToTouch = roomTouchCentre - currHandPos1;
//						prevHandToTouch = roomTouchCentre - prevHandPos1;
//
//						//set up the 2 vertices for a squre boundry for the gesture
//						glm::dvec3 upRight = glm::dvec3(initRoomTouchCentre.x+0.05, 0.0, initRoomTouchCentre.z+0.05);
//						glm::dvec3 lowLeft = glm::dvec3(initRoomTouchCentre.x-0.05, 0.0, initRoomTouchCentre.z-0.05);
//
//
//
//						double alpha = glm::dot(currHandToTouch,prevHandToTouch); // angle between both vectors
//
//						// get cross prod
//						glm::dvec3 cross = glm::cross(currHandToTouch, prevHandToTouch); 
//						glm::dvec3 normProjCross = glm::normalize(glm::dvec3(cross.x, 0.0, cross.z));
//
//
//						//std::cout << "Cross: " << glm::to_string(cross) << std::endl;
//
//						// project cross prod onto the screen, get a length
//						double lengthOfProjection = glm::dot(cross, normProjCross); 
//
//						// projected cross prod 
//						glm::dvec3 projectedCrossProd = lengthOfProjection * normProjCross; 
//
//						// modified angle that we rotate with
//						// 73.5 to make it more sensitive
//						alpha = 73.5 * alpha * lengthOfProjection;
//
//						// make a matrix transform, one for x rotation, one for z
//						// we're rotating around projectedCrossProd
//
//						glm::dmat4 XZRotMat = glm::rotate(glm::dmat4(1.0), alpha, normProjCross);
//
//
//						// put it into the matrix stack			
//						cFrameMgr->setRoomToVirtualSpaceFrame(cFrameMgr->getRoomToVirtualSpaceFrame() * XZRotMat);
//
//						/*glBegin(GL_LINES);
//							glVertex3f(roomTouchCentre.x, roomTouchCentre.y, roomTouchCentre.z);
//							glVertex3f(roomTouchCentre.x + projectedCrossProd.x, roomTouchCentre.y + projectedCrossProd.y, roomTouchCentre.z + projectedCrossProd.z);
//						glEnd();*/
//
//						//checking to see if there is at least 
//				
//
//						std::map<int, TouchDataRef>::iterator iter;
//						if(registeredTouchData.size() == 0){ //if no touch on screen then automatically exit the xzrot mode
//							xzRotFlag = false;
//							initRoomPos = true;
//							std::cout<<"no touchyyy so I quit"<<std::endl;
//						}
//						else{ //if there are touch(s) then check if the touch is in bound of the rectangle
//
//							for (iter = registeredTouchData.begin(); iter != registeredTouchData.end(); iter++) {
//						
//								// not exactly sure why roomPos.z > upRIght.z
//
//								if(!(iter->second->getCurrRoomPos().x > upRight.x || iter->second->getCurrRoomPos().z > upRight.z ||iter->second->getCurrRoomPos().x < lowLeft.x ||iter->second->getCurrRoomPos().z < lowLeft.z)){ //you are in the box
//							
//									std::cout << "fingers in bound so STILL IN XZRot Mode" << std::endl;
//									break;
//							
//						
//								}
//								if(iter == std::prev(registeredTouchData.end(),1)){
//									xzRotFlag = false;
//									initRoomPos = true;
//									std::cout << "all fingers went out of bound so Out of XZRot Mode" << std::endl;
//								}
//							}
//					
//						}
//
//
//
//				
//
//
//
//
//			
//					}
//		
//
//				}
//				//else if (name == "HandLeft"+closest) {
//				//	//TODO do something with the new handpos
//				//}
//			}
//		}
//	}
//}