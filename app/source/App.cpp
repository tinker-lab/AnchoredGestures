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

	cFrameMgr.reset(new CFrameMgr());
	//currentHCI.reset(new TuioHCI(window->getCamera(0), cFrameMgr,texMan));
	currentHCI.reset(new TestHCI(window->getCamera(0), cFrameMgr,texMan));

	initGL();
	initVBO(threadId);
	initLights();

	//glClearColor(0.f, 0.5f, 0.f, 0.f);

	

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}
	
	currentHCI->initializeContextSpecificVars(threadId, window);
}

glm::dvec3 getPosition(double latitude, double longitude) {
  // TOAD: Given a latitude and longitude as input, return the corresponding 3D x,y,z position 
  // on your Earth geometry
  
  // north pole is 0 deg latitude, 0 deg longitude
  // south pole is 180 lat, 0 longitude
  // longitude increase means counter clockwise, as viewed from north pole
  
  double latRad = glm::radians(latitude);
  double lonRad = glm::radians(longitude); 
  
  float z = sin(latRad)*cos(lonRad);
  float x = sin(latRad)*sin(lonRad);
  float y = cos(latRad);
  
  return glm::dvec3(x, y, z);
}

void App::initVBO(int threadId)
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


	

	//making tetrahedral axis head
	float const pi_OverTwelve = M_PI/12.0;

GLfloat tetraVertices[] = { 0.0f, 0.0f, 0.0f,  
								-0.4, 0.17*glm::cos(3*pi_OverTwelve), 0.17*glm::sin(3*pi_OverTwelve),  
								-0.4, -0.17*glm::cos(3*pi_OverTwelve), 0.17*glm::sin(3*pi_OverTwelve)
	}; 

	// normal array
	glm::dvec3 tetraNormals[]   = { 							
		glm::cross(glm::dvec3(0.0f,0.0f,0.0f)-glm::dvec3(-0.4,0.1*glm::cos(24*pi_OverTwelve)+0.07,0.1*glm::sin(24*pi_OverTwelve)+0.1),     glm::dvec3(0.0f,0.0f,0.0f)-glm::dvec3(-0.4,-0.1*glm::cos(24*pi_OverTwelve)-0.07,0.1*glm::sin(24*pi_OverTwelve)+0.1))
	};

	
	
	std::vector<int> tetraIndices;
	std::vector<GPUMesh::Vertex> tetraData;
	GPUMesh::Vertex tetraVert;
	for(int i=0; i<9; i = i+3){
		tetraVert.position = glm::dvec3(tetraVertices[i],tetraVertices[i+1],tetraVertices[i+2]);
		tetraVert.normal = tetraNormals[0];
		tetraVert.texCoord0 = glm::dvec2(colors[i],colors[i+1]);
		tetraData.push_back(tetraVert);
		tetraIndices.push_back(tetraData.size()-1);
		
	}

	tetraMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*tetraData.size(), sizeof(int)*tetraIndices.size(),0,tetraData,sizeof(int)*tetraIndices.size(), &tetraIndices[0]));

	
	//making axis body
	std::vector<int> axisIndices;
	std::vector<GPUMesh::Vertex> axisData;
	GPUMesh::Vertex axisVert;
	for(int i=0; i < 25; i++){
		axisVert.position = glm::dvec3(0.0f,0.1*glm::cos(i*pi_OverTwelve),0.1*glm::sin(i*pi_OverTwelve));
		axisVert.normal =glm::normalize(glm::dvec3(0.0f,glm::cos(i*pi_OverTwelve),glm::sin(i*pi_OverTwelve)));
		axisVert.texCoord0 = glm::dvec2(colors[i],colors[i+1]);
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);

		axisVert.position = glm::dvec3(0.8f,0.1*glm::cos(i*pi_OverTwelve),0.1*glm::sin(i*pi_OverTwelve));
		axisVert.normal =glm::normalize(glm::dvec3(0.0f,glm::cos(i*pi_OverTwelve),glm::sin(i*pi_OverTwelve)));
		axisVert.texCoord0 = glm::dvec2(colors[i],colors[i+1]);
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);
	}


	//initialize axisMesh Object
	axisMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*axisData.size(), sizeof(int)*axisIndices.size(), 0, axisData,sizeof(int)*axisIndices.size(), &axisIndices[0]));


	std::vector<int> sphereIndices;
	std::vector<GPUMesh::Vertex> sphereData;
	GPUMesh::Vertex sphereVert;

	// fill up the empty space
	const int STACKS = 40; // longitudes
	const int SLICES = 60; // latitudes
	const float latUnit = 180/STACKS;
	const float lonUnit = 360/SLICES;
	float curr_lat;
	float curr_lon;
	glm::dvec3 newVertex;
	glm::dvec3 nextVertex;
	glm::dvec3 newNormal; // can't create normals until we have 3 points...each vertex must have its own normal

	for (int i = 0; i < STACKS + 1; i++) { // stacks is outer loop
		curr_lat = i*latUnit;

		for (int k = 0; k < SLICES + 1; k++) {            
			curr_lon = k*lonUnit;

			//first vertex
			sphereVert.position = 0.1 * getPosition(curr_lat, curr_lon);
			sphereVert.normal = sphereVert.position;
			sphereVert.texCoord0 = glm::dvec2(colors[i],colors[i+1]);

			sphereData.push_back(sphereVert);
			sphereIndices.push_back(sphereData.size()-1);

			// second vertex
			sphereVert.position = 0.1 * getPosition(curr_lat + latUnit, curr_lon);
			sphereVert.normal = getPosition(curr_lat, curr_lon);
			sphereVert.texCoord0 = sphereVert.texCoord0;

			sphereData.push_back(sphereVert);
			sphereIndices.push_back(sphereData.size()-1);


			// Texture Coordinates
			/*cpuTexCoords.append(Vector2((1.0/SLICES)*k, (1.0/STACKS)*i));
			cpuTexCoords.append(Vector2((1.0/SLICES)*k, (1.0/STACKS)*(i+1)));*/
		}
	}

	sphereMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*sphereData.size(), sizeof(int)*sphereIndices.size(), 0, sphereData,sizeof(int)*sphereIndices.size(), &sphereIndices[0]));


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

void makeSphereMesh () {

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

    glClearColor(0, 0.3, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);
	
	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

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

void App::postInitialization() {
}

void App::drawGraphics(int threadId, MinVR::AbstractCameraRef camera,
		MinVR::WindowRef window) {
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR: "<<err<<std::endl;
	}

	currentHCI->draw(threadId,camera,window);

	const int numCubeIndices = (int)(cubeMesh->getFilledIndexByteSize()/sizeof(int));
	const int numAxisIndices = (int)(axisMesh->getFilledIndexByteSize()/sizeof(int));
	const int numSphereIndices = (int)(sphereMesh->getFilledIndexByteSize()/sizeof(int));
	const int numTetraIndices = (int)(tetraMesh->getFilledIndexByteSize()/sizeof(int));

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
	camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	MinVR::CameraOffAxis* offAxisCam = dynamic_cast<MinVR::CameraOffAxis*>(camera.get());

	shader->use();

	// need to put in variable for texture in shaders
	//shader->setUniform("lambertian_texture", 0); //lambertian_texture is the name of a sampler in glsl
	texMan->getTexture(threadId, "Koala1")->bind(0);

	shader->setUniform("projection_mat", offAxisCam->getLastAppliedProjectionMatrix());
	shader->setUniform("view_mat", offAxisCam->getLastAppliedViewMatrix());
	shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
	//shader->setUniform("normal_matrix", glm::dmat3(offAxisCam->getLastAppliedModelMatrix()));
	glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCam->getLastAppliedViewMatrix()), 3));
	shader->setUniform("eye_world", eye_world);

 //   glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
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


	// Begin drawing axes
	glBindVertexArray(axisMesh->getVAOID());
	// x-axis
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numAxisIndices);

	//// y-axis
	//glm::dmat4 yAxisRotMat = glm::rotate(glm::dmat4(1.0), 90.0, glm::dvec3(0.0, 0.0, 1.0));
	//glm::dmat4 yAxisAtCorner = (cFrameMgr->getVirtualToRoomSpaceFrame()*yAxisRotMat);
	//// yAxisAtCorner[3] 
	//camera->setObjectToWorldMatrix(yAxisAtCorner);
	//shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());

	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numAxisIndices);

	//// z-axis
	//glm::dmat4 zAxisRotMat = glm::rotate(glm::dmat4(1.0), 90.0, glm::dvec3(0.0, 1.0, 0.0));
	//glm::dmat4 zAxisAtCorner = (cFrameMgr->getVirtualToRoomSpaceFrame()*zAxisRotMat);
	////zAxisAtCorner[3] = glm::dvec4(0.3, 0.1, 0.0, 1.0); // modify fourth column
	//camera->setObjectToWorldMatrix(zAxisAtCorner);
	//shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());

	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numAxisIndices);

	//Draw sphere
	glBindVertexArray(sphereMesh->getVAOID());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numSphereIndices);

	// draw Arrow head
	glBindVertexArray(tetraMesh->getVAOID());
	// 4 faces
	for (int t = 0; t < 4; t++) {
		glm::dmat4 tetraRotMat1 = glm::rotate(glm::dmat4(1.0), t*90.0, glm::dvec3(1.0, 0.0, 0.0));
		glm::dmat4 tetraTran = (cFrameMgr->getVirtualToRoomSpaceFrame()*tetraRotMat1);
		tetraTran[3] = glm::dvec4(0.17, 0.1, 0.0, 1.0); // modify fourth column
		camera->setObjectToWorldMatrix( tetraTran );
		shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
		glDrawArrays(GL_TRIANGLES, 0, numTetraIndices);
	} 
	//Draw tetra face 1
	

	//// Draw tetra face 2
	//glm::dmat4 tetraRotMat1 = glm::rotate(glm::dmat4(1.0), 90.0, glm::dvec3(1.0, 0.0, 0.0));
	//

	//glDrawArrays(GL_TRIANGLES, 0, numTetraIndices);

	//// tetra face 3
	//tetraRotMat1 = glm::rotate(glm::dmat4(1.0), 180.0, glm::dvec3(1.0, 0.0, 0.0));
	//tetraTran = (cFrameMgr->getVirtualToRoomSpaceFrame()*tetraRotMat1);
	////tetraTran[3] = glm::dvec4(0.0, 0.1, 0.0, 1.0); // modify fourth column
	//camera->setObjectToWorldMatrix( tetraTran );
	//shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());

	//glDrawArrays(GL_TRIANGLES, 0, numTetraIndices);

	//tetraRotMat1 = glm::rotate(glm::dmat4(1.0), 270.0, glm::dvec3(1.0, 0.0, 0.0));
	//tetraTran = (cFrameMgr->getVirtualToRoomSpaceFrame()*tetraRotMat1);
	////tetraTran[3] = glm::dvec4(0.0, 0.1, 0.0, 1.0); // modify fourth column
	//camera->setObjectToWorldMatrix( tetraTran );
	//shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());

	//glDrawArrays(GL_TRIANGLES, 0, numTetraIndices);


}

