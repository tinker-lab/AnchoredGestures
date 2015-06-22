#ifndef AXIS_H_
#define AXIS_H_

#include "app/include/GPUMesh.h"
#include <GLFW/glfw3.h>
#include "MVRCore/Event.H"
#include <glm/glm.hpp>
#include <memory>
#include <glm/gtx/string_cast.hpp>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef std::shared_ptr<class Axis> AxisRef;

class Axis {

public:
	Axis();
	virtual ~Axis();
	void draw();
	glm::dvec3 getPosition(double latitude, double longitude);

private:
	std::shared_ptr<GPUMesh> axisMesh;
	std::shared_ptr<GPUMesh> sphereMesh;
	
};

#endif /* AXIS_H_ */