#include "app/include/Feedback.h"

Feedback::Feedback(MinVR::AbstractCameraRef camera,CFrameMgrRef cFrameMgr,TextureMgrRef texMan) {
	this->cFrameMgr = cFrameMgr;
	this->texMan = texMan;
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	displayText = "";
}

Feedback::~Feedback() {
}


void Feedback::initializeContextSpecificVars(int threadId,MinVR::WindowRef window) {
	
	initVBO(threadId, window);
	initGL();

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<< err << std::endl;
	}
}

void Feedback::initVBO(int threadId, MinVR::WindowRef window) {

	std::vector<int> quadIndices;
	std::vector<GPUMesh::Vertex> quadData;
	GPUMesh::Vertex quadVert;

	float windowHeight = window->getHeight();
	float windowWidth = window->getWidth();
	float texHeight = texMan->getTexture(threadId, "rotating")->getHeight();
	float texWidth = texMan->getTexture(threadId, "rotating")->getWidth();
	float quadHeightScreen =  texHeight/windowHeight;
	float quadWidthScreen = texWidth/windowWidth;

	glm::dvec3 quad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(quadWidthScreen+0.5, quadHeightScreen+0.5)));

	std::cout<<"quad: "<<glm::to_string(quad)<<std::endl;
	std::cout<<"wind H: "<<windowHeight<<std::endl;
	std::cout<<"wind W: "<<windowWidth<<std::endl;
	std::cout<<"tex H: "<<texHeight<<std::endl;
	std::cout<<"tex W: "<<texWidth<<std::endl;

	std::cout << "quad Height Screen: " <<quadHeightScreen << std::endl;
	std::cout << "quad W Screen: " << quadWidthScreen << std::endl;

	glm::dvec3 topleft = convertScreenToRoomCoordinates(glm::dvec2(0.1,0.9));
	std::cout<<"topleft: "<<glm::to_string(topleft)<<std::endl;

	quadVert.position = glm::dvec3(topleft.x, 0.0, topleft.z);
	quadVert.normal = glm::dvec3(0.0, 1.0, 0.0);
	quadVert.texCoord0 = glm::dvec2(0.0, 1.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);

	quadVert.position = glm::dvec3(topleft.x, 0.0, topleft.z+quad.z);
	quadVert.texCoord0 = glm::dvec2(0.0, 0.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);

	quadVert.position = glm::dvec3(topleft.x+quad.x, 0.0, topleft.z);
	quadVert.texCoord0 = glm::dvec2(1.0, 1.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);


	quadVert.position = glm::dvec3(topleft.x+quad.x, 0.0, topleft.z+quad.z);
	quadVert.texCoord0 = glm::dvec2(1.0, 0.0);
	quadData.push_back(quadVert);
	quadIndices.push_back(quadData.size()-1);


	quadMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*quadData.size(), sizeof(int)*quadIndices.size(),0,quadData,sizeof(int)*quadIndices.size(), &quadIndices[0]));
}

void Feedback::initGL() {

	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	// Wait, do we even need to load shaders?
	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tex.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("tex.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

}

glm::dvec3 Feedback::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}

void Feedback::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window) {

	// if we are in some mode, displayText should be fine
	// displayText is modified through HCIs
	if (displayText != "") {
		shader->use();
		shader->setUniform("projection_mat", offAxisCamera->getLastAppliedProjectionMatrix());
		shader->setUniform("view_mat", offAxisCamera->getLastAppliedViewMatrix());

		texMan->getTexture(threadId, displayText)->bind(1);
		shader->setUniform("textureSampler", 1);

		glm::dvec4 quadTranslate(0.0, 0.0, 0.0, 1.0);
		const int numQuadIndices = (int)(quadMesh->getFilledIndexByteSize()/sizeof(int));

		//turn on blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// draw text here, remember to mess with the shader with the alpha value
		glm::dmat4 quadAtCorner = glm::dmat4(1.0);
		quadAtCorner[3] = quadTranslate;
		camera->setObjectToWorldMatrix(quadAtCorner);
		shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
		glBindVertexArray(quadMesh->getVAOID());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuadIndices);

		// turn off blending
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);

		//   glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
		/*glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);*/
	}

}

