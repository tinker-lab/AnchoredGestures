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
	std::cout<<"TuioHCIinit is been called"<<std::endl;

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}

	cFrameMgr.reset(new CFrameMgr());
	
}

void TuioHCI::initVBO(int threadId) {
	GLfloat vertices[]  = { 0.25f, 0.0f, 0.25f,  -0.25f, 0.0f, -0.25f,  -0.25f, 0.0f, 0.25f};
	GLfloat normals[]   = { 0, 1, 0,   0, 1, 0,   0, 1, 0};
	
	std::vector<int> cubeIndices;
	std::vector<GPUMesh::Vertex> cubeData;
	GPUMesh::Vertex vert;
	for(int i=0; i < 9; i = i +3){
		vert.position = glm::dvec3(vertices[i],vertices[i+1],vertices[i+2]);
		vert.normal = glm::normalize(glm::dvec3(normals[i],normals[i+1],normals[i+2]));
		vert.texCoord0 = glm::dvec2(0.1,0.9);
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
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

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
			//cubeMesh->updateVertexData(int startByteOffset, int vertexOffset, const std::vector<Vertex> &data);

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



void TuioHCI::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window){

	// want to draw for each event.
	//std::map<int,MinVR::EventRef>::iterator it;
	//for (it = registeredEvents.begin(); it != registeredEvents.end(); ++it) {
	//	//draw each events
	//}
	
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
	glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCam->getLastAppliedViewMatrix()), 3));
	shader->setUniform("eye_world", eye_world);

	std::map<int,MinVR::EventRef>::iterator it;
	glm::dvec3 roomCoord;

	glBindVertexArray(cubeMesh->getVAOID());

	for(it = registeredEvents.begin(); it != registeredEvents.end(); ++it) {
		// new matrix for each triangle
		MinVR::EventRef event = it->second;
		
		
		// only two events in map: a move and a down
		if (boost::algorithm::starts_with(event->getName(), "TUIO_Cursor_down")) {
			roomCoord = convertScreenToRoomCoordinates(event->get2DData());
		} else { // move event
			roomCoord = convertScreenToRoomCoordinates( glm::dvec2(event->get4DData()) );
		}
		
		camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), roomCoord));
		//camera->setObjectToWorldMatrix(glm::dmat4(1.0f));
		shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
		// draw triangl
		std::cout << roomCoord.x << ", " << roomCoord.y << ", " << roomCoord.z << std::endl;
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
	}
	
	
	// need to put in variable for texture in shaders
	//shader->setUniform("lambertian_texture", 0); //lambertian_texture is the name of a sampler in glsl
	//texMan->getTexture(threadId, "Koala")->bind(0);

}

glm::dvec3 TuioHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}