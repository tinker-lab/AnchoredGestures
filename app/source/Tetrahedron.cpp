#include "app/includes/Tetrahedron.h"

Tetrahedron::Tetrahedron() {

}

Tetrahedron::~Tetrahedron() {
}

void initializeContextSpecificVars(int threadId,MinVR::WindowRef window){
	GPUcylinderOffset = 25;
}

void Tetrahedron::initVBO(int threadId) {
	// modifies the cylinder and sphere GPUMeshes
	makeCylinder(glm::dvec3 pointA, glm::dvec3 pointB);



	//initialize cylinderMesh Object after all the cylinder points are push into mesh
	cylinderMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*axisData.size(), sizeof(int)*axisIndices.size(), 0, axisData,sizeof(int)*axisIndices.size(), &axisIndices[0]));


	makeSphere(glm::dvec3 center);
	
	//initialize sphereMesh Object after all the sphere points are push into mesh
	sphereMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*sphereData.size(), sizeof(int)*sphereIndices.size(), 0, sphereData,sizeof(int)*sphereIndices.size(), &sphereIndices[0]));



}

void Tetrahedron::initGL() {

	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	tetraShader.reset(new GLSLProgram());
	tetraShader->compileShader(MinVR::DataFileUtils::findDataFile("tex.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	tetraShader->compileShader(MinVR::DataFileUtils::findDataFile("tex.frag").c_str(), GLSLShader::FRAGMENT, args);
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


void makeCylinder(glm::dvec3 pointA, glm::dvec3 pointB){
//making cylinder body
	std::vector<int> axisIndices;
	std::vector<GPUMesh::Vertex> axisData;
	GPUMesh::Vertex axisVert;
	for(int i=0; i < GPUcylinderOffset; i++){
		axisVert.position = glm::dvec3(0.0f,0.1*glm::cos(i*piTwelfths),0.1*glm::sin(i*piTwelfths));
		axisVert.normal =glm::normalize(glm::dvec3(0.0f,glm::cos(i*piTwelfths),glm::sin(i*piTwelfths)));
		axisVert.texCoord0 = glm::dvec2(0.0, 0.0);
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);

		axisVert.position = glm::dvec3(0.8f,0.1*glm::cos(i*piTwelfths),0.1*glm::sin(i*piTwelfths));
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);
	}


	

}

void makeSphere(glm::dvec3 center){


std::vector<int> sphereIndices;
	std::vector<GPUMesh::Vertex> sphereData;
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
			sphereVert.position = 0.1 * getPosition(curr_lat, curr_lon);
			sphereVert.normal = sphereVert.position;
			sphereVert.texCoord0 = glm::dvec2(0.5,0.5);

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

	
}

void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window, std::string textureName){
	
	const int numCylinderIndices = (int)(cylinderMesh->getFilledIndexByteSize()/sizeof(int));
	const int numSphereIndices = (int)(sphereMesh->getFilledIndexByteSize()/sizeof(int));

	tetraShader->use();
	tetraShader->setUniform("projection_mat", offAxisCamera->getLastAppliedProjectionMatrix());
	tetraShader->setUniform("view_mat", offAxisCamera->getLastAppliedViewMatrix());
	
	glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCamera->getLastAppliedViewMatrix()), 3));
	tetraShader->setUniform("eye_world", eye_world);
	texMan->getTexture(threadId, textureName)->bind(0);
	tetraShader->setUniform("textureSampler", 0);

	// Begin drawing cylinder
	glBindVertexArray(cylinderMesh->getVAOID());

	// cylinders
	camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());
	tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	for(int c = 0; c < 6 ; c++){
	glDrawArrays(GL_TRIANGLE_STRIP, c*GPUcylinderOffset, (c+1)*GPUcylinderOffset);
	}

	//Draw sphere
	glBindVertexArray(sphereMesh->getVAOID());


	// 4 spheres
	for (int t = 0; t < 4; t++) {
		glm::dmat4 sphereTransMat1 = glm::translate();//fill in later
		tetraShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix()*sphereTransMat1);
		glDrawArrays(GL_TRIANGLES, 0, numSphereIndices);
	} 
}


