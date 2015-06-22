#include "app/include/Axis.h"

const double piTwelfths = M_PI/12.0;

Axis::Axis() {

	//making axis body
	std::vector<int> axisIndices;
	std::vector<GPUMesh::Vertex> axisData;
	GPUMesh::Vertex axisVert;
	for(int i=0; i < 25; i++){
		axisVert.position = glm::dvec3(0.0f,0.1*glm::cos(i*piTwelfths),0.1*glm::sin(i*piTwelfths));
		axisVert.normal =glm::normalize(glm::dvec3(0.0f,glm::cos(i*piTwelfths),glm::sin(i*piTwelfths)));
		axisVert.texCoord0 = glm::dvec2(0.0, 0.0);
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);

		axisVert.position = glm::dvec3(0.8f,0.1*glm::cos(i*piTwelfths),0.1*glm::sin(i*piTwelfths));
		axisData.push_back(axisVert);
		axisIndices.push_back(axisData.size()-1);
	}


	//initialize axisMesh Object
	axisMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*axisData.size(), sizeof(int)*axisIndices.size(), 0, axisData,sizeof(int)*axisIndices.size(), &axisIndices[0]));


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
			sphereVert.texCoord0 = glm::dvec2(0.0,0.0);

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

}

Axis::~Axis() {

}

glm::dvec3 Axis::getPosition(double latitude, double longitude) {
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

void Axis::draw(){
	
}