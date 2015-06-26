#include <app/include/App.h>



using namespace MinVR;

App::App() : MinVR::AbstractMVRApp() {

}

App::~App() {
	for(std::map<int, GLuint>::iterator iterator = _vboId.begin(); iterator != _vboId.end(); iterator++) {
		glDeleteBuffersARB(1, &iterator->second);
	}
}

void App::doUserInputAndPreDrawComputation(
	const std::vector<MinVR::EventRef>& events, double synchronizedTime) {
	for(int i=0; i < events.size(); i++) {
		if (events[i]->getName() == "kbd_ESC_down") {
			exit(0);
		}

		if (events[i]->getName() == "kbd_SPACE_down") {
			cFrameMgr->setRoomToVirtualSpaceFrame(glm::dmat4(1.0)); 
		}

	}
	
	currentHCI->update(events);
}

void App::initializeContextSpecificVars(int threadId,
		MinVR::WindowRef window) {

	texMan.reset(new TextureMgr());
	texMan->loadTexturesFromConfigVal(threadId, "LoadTextures");

	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(window->getCamera(0));

	cFrameMgr.reset(new CFrameMgr());
	feedback.reset(new Feedback(window->getCamera(0), cFrameMgr, texMan));

	currentHCI.reset(new TestHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	axis.reset(new Axis(window->getCamera(0), cFrameMgr, texMan));

	experimentMgr.reset(new ExperimentMgr(currentHCI));
	

	initGL();
	initVBO(threadId, window);
	initLights();

	axis->initializeContextSpecificVars(threadId, window);
	feedback->initializeContextSpecificVars(threadId, window);
	//glClearColor(0.f, 0.5f, 0.f, 0.f);

	
	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}
	
	currentHCI->initializeContextSpecificVars(threadId, window);
}


void App::initVBO(int threadId, MinVR::WindowRef window)
{
	// cube ///////////////////////////////////////////////////////////////////////
	//    v6----- v5
	//   /|      /|
	//  v1------v0|
	//  | |     | |
	//  | |v7---|-|v4
	//  |/      |/
	//  v2------v3

	// vertex coords array for glDrawArrays() =====================================
	// A cube has 6 sides and each side has 2 triangles, therefore, a cube consists
	// of 36 vertices (6 sides * 2 tris * 3 vertices = 36 vertices). And, each
	// vertex is 3 components (x,y,z) of floats, therefore, the size of vertex
	// array is 108 floats (36 * 3 = 108).
	GLfloat vertices[]  = { 1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f,-1.0f, 1.0f,      // v0-v1-v2 (front)
						   -1.0f,-1.0f, 1.0f,   1.0f,-1.0f, 1.0f,   1.0f, 1.0f, 1.0f,      // v2-v3-v0

							1.0f, 1.0f, 1.0f,   1.0f,-1.0f, 1.0f,   1.0f,-1.0f,-1.0f,      // v0-v3-v4 (right)
							1.0f,-1.0f,-1.0f,   1.0f, 1.0f,-1.0f,   1.0f, 1.0f, 1.0f,      // v4-v5-v0

							1.0f, 1.0f, 1.0f,   1.0f, 1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,      // v0-v5-v6 (top)
						   -1.0f, 1.0f,-1.0f,  -1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,      // v6-v1-v0

						   -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f,-1.0f,  -1.0f,-1.0f,-1.0f,      // v1-v6-v7 (left)
						   -1.0f,-1.0f,-1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,      // v7-v2-v1.0

						   -1.0f,-1.0f,-1.0f,   1.0f,-1.0f,-1.0f,   1.0f,-1.0f, 1.0f,      // v7-v4-v3 (bottom)
							1.0f,-1.0f, 1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f,-1.0f,-1.0f,      // v3-v2-v7

							1.0f,-1.0f,-1.0f,  -1.0f,-1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,      // v4-v7-v6 (back)
						   -1.0f, 1.0f,-1.0f,   1.0f, 1.0f,-1.0f,   1.0f,-1.0f,-1.0f };    // v6-v5-v4

	// normal array
	GLfloat normals[]   = { 0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
							0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0

							1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
							1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0

							0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
							0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0

						   -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
						   -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1

							0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
							0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7

							0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
							0, 0,-1,   0, 0,-1,   0, 0,-1		// v6-v5-v4
	};

	// color array
	GLfloat colors[]    = { 1, 1, 1,   1, 1, 0,   1, 0, 0,      // v0-v1-v2 (front)
							1, 0, 0,   1, 0, 1,   1, 1, 1,      // v2-v3-v0

							1, 1, 1,   1, 0, 1,   0, 0, 1,      // v0-v3-v4 (right)
							0, 0, 1,   0, 1, 1,   1, 1, 1,      // v4-v5-v0

							1, 1, 1,   0, 1, 1,   0, 1, 0,      // v0-v5-v6 (top)
							0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

							1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
							0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

							0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
							1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

							0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
							0, 1, 0,   0, 1, 1,   0, 0, 1,


							0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

							1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
							0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

							0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
							1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

							0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
							0, 1, 0,   0, 1, 1,   0, 0, 1};    // v6-v5-v4

	//making cubeData
	std::vector<int> cubeIndices;
	std::vector<GPUMesh::Vertex> cubeData;
	GPUMesh::Vertex vert;
	for(int i=0; i < 108; i = i +3){
		vert.position = glm::dvec3(vertices[i],vertices[i+1],vertices[i+2]);
		vert.normal = glm::normalize(glm::dvec3(normals[i],normals[i+1],normals[i+2]));
		vert.texCoord0 = glm::dvec2(colors[i],colors[i+1]);
		cubeData.push_back(vert);
		cubeIndices.push_back(cubeData.size()-1);
	}
	
	//initialize Mesh Object

	cubeMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*cubeData.size(), sizeof(int)*cubeIndices.size(),0,cubeData,sizeof(int)*cubeIndices.size(), &cubeIndices[0]));

	// for the background
	std::vector<int> bgQuadIndices;
	std::vector<GPUMesh::Vertex> bgQuadData;
	GPUMesh::Vertex bgQuadVert;

	float windowHeight = window->getHeight();
	float windowWidth = window->getWidth();

	glm::dvec3 bgQuad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(1.0, 1.0))); // fill up screen size

	bgQuadVert.position = glm::dvec3(-bgQuad.x, 0.0, -bgQuad.z);
	bgQuadVert.normal = glm::dvec3(0.0, 1.0, 0.0);
	bgQuadVert.texCoord0 = glm::dvec2(0.0, 1.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);

	bgQuadVert.position = glm::dvec3(-bgQuad.x, 0.0, bgQuad.z);
	bgQuadVert.texCoord0 = glm::dvec2(0.0, 0.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);

	bgQuadVert.position = glm::dvec3(bgQuad.x, 0.0, -bgQuad.z);
	bgQuadVert.texCoord0 = glm::dvec2(1.0, 1.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);


	bgQuadVert.position = glm::dvec3(bgQuad.x, 0.0, bgQuad.z);
	bgQuadVert.texCoord0 = glm::dvec2(1.0, 0.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);

	bgMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*bgQuadData.size(), sizeof(int)*bgQuadIndices.size(),0,bgQuadData,sizeof(int)*bgQuadIndices.size(), &bgQuadIndices[0]));

	//axis->initVBO(0);
	//feedback->initVBO(threadId, window);

    // create vertex buffer objects, you need to delete them when program exits
    // Try to put both vertex coords array, vertex normal array and vertex color in the same buffer object.
    // glBufferDataARB with NULL pointer reserves only memory space.
    // Copy actual data with 2 calls of glBufferSubDataARB, one for vertex coords and one for normals.
    // target flag is GL_ARRAY_BUFFER_ARB, and usage flag is GL_STATIC_DRAW_ARB
	_vboId[threadId] = GLuint(0);
	//glGenBuffersARB(1, &_vboId[threadId]);
 //   glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vboId[threadId]);
 //   glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_STATIC_DRAW_ARB);
 //   glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(vertices), vertices);                             // copy vertices starting from 0 offest
 //   glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices), sizeof(normals), normals);                // copy normals after vertices
 //   glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);  // copy colours after normals

	
	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initVBO: "<<err<<std::endl;
	}
}



void App::initGL()
{
	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_CULL_FACE);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0.0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);
	
	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

	bgShader.reset(new GLSLProgram());
	bgShader->compileShader(MinVR::DataFileUtils::findDataFile("tex.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	bgShader->compileShader(MinVR::DataFileUtils::findDataFile("tex.frag").c_str(), GLSLShader::FRAGMENT, args);
	bgShader->link();

	//initGL for the HCIs
	currentHCI->initGL();


	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initGL: "<<err<<std::endl;
	}
}

void App::initLights()
{
	// set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0.5, 0, 3, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initLights: "<<err<<std::endl;
	}
}

glm::dvec3 App::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}

void App::postInitialization() {
}

void App::drawGraphics(int threadId, MinVR::AbstractCameraRef camera,
		MinVR::WindowRef window) {
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR: "<<err<<std::endl;
	}

	

	const int numCubeIndices = (int)(cubeMesh->getFilledIndexByteSize()/sizeof(int));
	const int numBgQuadIndices = (int)(bgMesh->getFilledIndexByteSize()/sizeof(int));
	//glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vboId[threadId]);

    // enable vertex arrays
  /*  glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);*/

    // before draw, specify vertex and index arrays with their offsets
 //   glNormalPointer(GL_FLOAT, 0, (void*)(sizeof(GLfloat)*108));
 //   glColorPointer(3, GL_FLOAT, 0, (void*)((sizeof(GLfloat)*108)+(sizeof(GLfloat)*108)));
 //   glVertexPointer(3, GL_FLOAT, 0, 0);

	//glm::dmat4 translate = glm::translate(glm::dmat4(1.0f), glm::dvec3(0.0f, 0.0f, -5.0f));
	//glm::dvec2 rotAngles(-20.0, 45.0);
	//glm::dmat4 rotate1 = glm::rotate(translate, rotAngles.y, glm::dvec3(0.0,1.0,0.0));
	//camera->setObjectToWorldMatrix(glm::rotate(rotate1, rotAngles.x, glm::dvec3(1.0,0,0)));
	/*camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());*/
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	MinVR::CameraOffAxis* offAxisCam = dynamic_cast<MinVR::CameraOffAxis*>(camera.get());

	/////////////////////////////
	// Draw Background Picture //
	/////////////////////////////
	bgShader->use();
	bgShader->setUniform("projection_mat", offAxisCam->getLastAppliedProjectionMatrix());
	bgShader->setUniform("view_mat", offAxisCam->getLastAppliedViewMatrix());
	bgShader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
	texMan->getTexture(threadId, "back1")->bind(4);
	bgShader->setUniform("textureSampler", 4);
	camera->setObjectToWorldMatrix(glm::dmat4(1.0));
	bgShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	glBindVertexArray(bgMesh->getVAOID());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numBgQuadIndices);

	
	
	

	/////////////////////////////
	// General Things          //
	/////////////////////////////
	shader->use();
	shader->setUniform("projection_mat", offAxisCam->getLastAppliedProjectionMatrix());
	shader->setUniform("view_mat", offAxisCam->getLastAppliedViewMatrix());
	shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
	//shader->setUniform("normal_matrix", glm::dmat3(offAxisCam->getLastAppliedModelMatrix()));
	glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCam->getLastAppliedViewMatrix()), 3));
	shader->setUniform("eye_world", eye_world);

	/////////////////////////////
	// Draw Cube               //
	/////////////////////////////
	camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());
	shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	texMan->getTexture(threadId, "Koala1")->bind(0);
	glBindVertexArray(cubeMesh->getVAOID());
	glDrawArrays(GL_TRIANGLES, 0, numCubeIndices);

	/////////////////////////////
	// Draw Current HCI Stuff  //
	/////////////////////////////
	currentHCI->draw(threadId,camera,window);	

	/////////////////////////////
	// Draw Axes               // // These use the tex shader, not the phong shaders
	/////////////////////////////
	glm::dvec4 cornerTranslate(-1.7, 0.0, 0.95, 1.0); // modify fourth column
	glm::dmat4 scaleAxisMat = glm::scale(
			glm::dmat4(1.0),
			glm::dvec3(0.1)); 

	//draw x axis
	glm::dmat4 xAxisAtCorner = cFrameMgr->getVirtualToRoomSpaceFrame() * scaleAxisMat;
	xAxisAtCorner[3] = cornerTranslate; // modify fourth column
	camera->setObjectToWorldMatrix(xAxisAtCorner);
	axis->draw(threadId, camera, window, "red");
	
	//draw y axis
	glm::dmat4 yAxisRotMat = glm::rotate(glm::dmat4(1.0), 90.0, glm::dvec3(0.0, 0.0, 1.0));
	glm::dmat4 yAxisAtCorner = (cFrameMgr->getVirtualToRoomSpaceFrame()* scaleAxisMat *yAxisRotMat);
	yAxisAtCorner[3] = cornerTranslate; // modify fourth column
	camera->setObjectToWorldMatrix(yAxisAtCorner);
	axis->draw(threadId, camera, window, "green");

	//// z-axis
	glm::dmat4 zAxisRotMat = glm::rotate(glm::dmat4(1.0), -90.0, glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 zAxisAtCorner = (cFrameMgr->getVirtualToRoomSpaceFrame() * scaleAxisMat * zAxisRotMat);
	zAxisAtCorner[3] = cornerTranslate; // modify fourth column
	camera->setObjectToWorldMatrix(zAxisAtCorner);
	axis->draw(threadId, camera, window, "blue");

	// draw text (actually just a texture)
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	

	/////////////////////////////
	// Draw Visual Feedback    //
	/////////////////////////////
	feedback->draw(threadId, camera, window);

	

 ////   glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
 //   glDisableClientState(GL_COLOR_ARRAY);
 //   glDisableClientState(GL_NORMAL_ARRAY);

 //   glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	/*
	camera->setObjectToWorldMatrix(glm::mat4(1.0));
	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-0.3f, -0.2f, -1.f);
	glColor3f(0.f, 1.0f, 0.f);
	glVertex3f(0.3f, -0.2f, -1.0f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.3f, -1.f);
	glEnd();
	*/
	//glBindVertexArray(cubeMesh->getVAOID());
	//glDrawArrays(GL_TRIANGLES, 0, numCubeIndices);


	

}

