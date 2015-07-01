#include "app/include/Tetrahedron.h"

Tetrahedron::Tetrahedron(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan) {
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;
	this->cFrameMgr = cFrameMgr;
}

Tetrahedron::~Tetrahedron() {
}

void Tetrahedron::initializeContextSpecificVars(int threadId) {
	GPUcylinderOffset = 50; // had it at 25 before
	initVBO(threadId);
	initGL();
}

void Tetrahedron::initVBO(int threadId) {

	glm::dvec3 makeOriginAsCentroid (0.0, -0.1875, 0.05); // looks messy, I know
	// modifies the cylinder and sphere GPUMeshes
	//-------------------first cylinder-------------------------
	pointA = -makeOriginAsCentroid + glm::dvec3(0.2, -0.3, 0.2)*1.25; //a
	pointB = -makeOriginAsCentroid + glm::dvec3(-0.2, -0.3, 0.2)*1.25; //b
	pointC = -makeOriginAsCentroid + glm::dvec3(0.0, -0.3, -0.2)*1.25; //c
	pointD = -makeOriginAsCentroid + glm::dvec3(0.0, 0.15, 0.0)*1.25; //d

	makeCylinder(pointA, pointB);

	//-------------------second cylinder-------------------------
	
	makeCylinder(pointA, pointC);

	//-------------------third cylinder-------------------------
	makeCylinder(pointB, pointC);

	//-------------------fourth cylinder-------------------------
	
	makeCylinder(pointA, pointD);

	//-------------------fifth cylinder-------------------------

	makeCylinder(pointB, pointD);

	//-------------------sixth cylinder-------------------------

	makeCylinder(pointC, pointD);

	//initialize cylinderMesh Object after all the cylinder points are push into mesh
	cylinderMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*cylinderData.size(), sizeof(int)*cylinderIndices.size(), 0, cylinderData,sizeof(int)*cylinderIndices.size(), &cylinderIndices[0]));

	//make one sphere then translate it
	glm::dvec3 center = glm::dvec3(0.0,0.0,0.0);
	makeSphere(center);
	
	//initialize sphereMesh Object after all the sphere points are push into mesh
	sphereMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*sphereData.size(), sizeof(int)*sphereIndices.size(), 0, sphereData,sizeof(int)*sphereIndices.size(), &sphereIndices[0]));



}

void Tetrahedron::initGL() {

	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	tetraShader.reset(new GLSLProgram());
	tetraShader->compileShader(MinVR::DataFileUtils::findDataFile("phong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	tetraShader->compileShader(MinVR::DataFileUtils::findDataFile("fong.frag").c_str(), GLSLShader::FRAGMENT, args);
	tetraShader->link();

}


glm::dvec3 Tetrahedron::getPosition(double latitude, double longitude) {
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


void Tetrahedron::makeCylinder(glm::dvec3 pointA, glm::dvec3 pointB){

	double piTwelfths = (M_PI/12.0);

	//making cylinder body

	GPUMesh::Vertex cylinderVert;
	glm::dvec3 mainVector = pointB - pointA; 
	glm::dvec3 normRvec = glm::normalize(glm::cross(mainVector,glm::dvec3(0.0, 1.0, 0.0)));
	glm::dvec3 normNvec = glm::normalize(glm::cross(mainVector, normRvec));


	for(int i=0; i < 25; i++){
		cylinderVert.position = pointB + normRvec*0.02*glm::cos(i*piTwelfths)+ normNvec*0.02*glm::sin(i*piTwelfths);
		cylinderVert.normal = glm::normalize(normRvec*glm::cos(i*piTwelfths)+ normNvec*glm::sin(i*piTwelfths));
		cylinderVert.texCoord0 = glm::dvec2(0.2, 0.4);
		cylinderData.push_back(cylinderVert);
		cylinderIndices.push_back(cylinderData.size()-1);

		cylinderVert.position = pointA + normRvec*0.02*glm::cos(i*piTwelfths)+ normNvec*0.02*glm::sin(i*piTwelfths);
		cylinderData.push_back(cylinderVert);
		cylinderIndices.push_back(cylinderData.size()-1);
	}


	

}

void Tetrahedron::makeSphere(glm::dvec3 center){



	GPUMesh::Vertex sphereVert;

	// fill up the empty space
	const float STACKS = 40.0f; // longitudes
	const float SLICES = 60.0f; // latitudes
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
			sphereVert.position = 0.04 * getPosition(curr_lat, curr_lon);
			sphereVert.normal = sphereVert.position;
			sphereVert.texCoord0 = glm::dvec2(0.5,0.5);

			sphereData.push_back(sphereVert);
			sphereIndices.push_back(sphereData.size()-1);

			// second vertex
			sphereVert.position = 0.04 * getPosition(curr_lat + latUnit, curr_lon);
			sphereVert.normal = getPosition(curr_lat, curr_lon);
			sphereVert.texCoord0 = sphereVert.texCoord0;

			sphereData.push_back(sphereVert);
			sphereIndices.push_back(sphereData.size()-1);


			// Texture Coordinates
			/*cpuTexCoords.append(Vector2((1.0/SLICES)*k, (1.0/STACKS)*i));
			cpuTexCoords.append(Vector2((1.0/SLICES)*k, (1.0/STACKS)*(i+1)));*/
		}
	}

	
}

void Tetrahedron::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window, std::string textureName, glm::dmat4 transMat){
	
	const int numCylinderIndices = (int)(cylinderMesh->getFilledIndexByteSize()/sizeof(int));
	const int numSphereIndices = (int)(sphereMesh->getFilledIndexByteSize()/sizeof(int));

	tetraShader->use();
	tetraShader->setUniform("projection_mat", offAxisCamera->getLastAppliedProjectionMatrix());
	tetraShader->setUniform("view_mat", offAxisCamera->getLastAppliedViewMatrix());

	//glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCamera->getLastAppliedViewMatrix()), 3));
	//tetraShader->setUniform("eye_world", eye_world);
	texMan->getTexture(threadId, textureName)->bind(6);
	tetraShader->setUniform("textureSampler", 6);

	////////////////////////
	// static tetrahedron //
	////////////////////////
	glBindVertexArray(cylinderMesh->getVAOID());
	// tetraPosition will be loaded in from a config file later
	//glm::dmat4 tetraPosition = glm::translate(glm::dmat4(1.0),glm::dvec3(0.0, 0.0, 0.0));
	camera->setObjectToWorldMatrix(transMat);
	tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	for(int c = 0; c < 6 ; c++) {
		//std::cout << "The indexes for drawing: " << c * GPUcylinderOffset << ", " << (c+1) * GPUcylinderOffset << std::endl;
		//std::cout << 0 << ", " << numCylinderIndices << std::endl;
		glDrawArrays(GL_TRIANGLE_STRIP, c*GPUcylinderOffset, GPUcylinderOffset);
	}

	///////////////////////////////
	// transformable tetrahedron //
	///////////////////////////////
	camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());
	tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	for(int c = 0; c < 6 ; c++) {
		//std::cout << "The indexes for drawing: " << c * GPUcylinderOffset << ", " << (c+1) * GPUcylinderOffset << std::endl;
		//std::cout << 0 << ", " << numCylinderIndices << std::endl;
		glDrawArrays(GL_TRIANGLE_STRIP, c*GPUcylinderOffset, GPUcylinderOffset);
	}


	///////////////////////////////
	// Draw Tetrahedron spheres  //
	///////////////////////////////
	glBindVertexArray(sphereMesh->getVAOID());
	texMan->getTexture(threadId, "red")->bind(7);
	tetraShader->setUniform("textureSampler", 7);

	glm::dmat4 sphereTransMat1 = glm::translate(glm::dmat4(1.0), pointA);
	tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix()*sphereTransMat1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numSphereIndices);

	texMan->getTexture(threadId, "green")->bind(7);
	tetraShader->setUniform("textureSampler", 7);
	glm::dmat4 sphereTransMat2 = glm::translate(glm::dmat4(1.0), pointB);
	tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix()*sphereTransMat2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numSphereIndices);

	texMan->getTexture(threadId, "blue")->bind(7);
	tetraShader->setUniform("textureSampler", 7);
	glm::dmat4 sphereTransMat3 = glm::translate(glm::dmat4(1.0), pointC);
	tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix()*sphereTransMat3);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numSphereIndices);

	texMan->getTexture(threadId, "Koala")->bind(7);
	tetraShader->setUniform("textureSampler", 7);
	glm::dmat4 sphereTransMat4 = glm::translate(glm::dmat4(1.0), pointD);
	tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix()*sphereTransMat4);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numSphereIndices);
	
	// 4 spheres
	//for (int t = 0; t < 4; t++) {
	//	glm::dmat4 sphereTransMat1 = glm::translate(glm::dmat4(1.0), pointA);
	//	//tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix()*sphereTransMat1);
	//	glDrawArrays(GL_TRIANGLES, 0, numSphereIndices);
	//} 


	
}


