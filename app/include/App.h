#ifndef APP_H_
#define APP_H_

#include "GL/glew.h"
#include "MVRCore/AbstractMVRApp.H"
#include "MVRCore/AbstractCamera.H"
#include "MVRCore/AbstractWindow.H"
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "MVRCore/Event.H"
#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include "app/include/GPUMesh.h"
#include "app/include/GLSLProgram.h"
#include "app/include/TuioHCI.h"
#include "app/include/TestHCI.h"
#include "app/include/AbstractHCI.h"
#include "app/include/TextureMgr.h"
#include "app/include/TouchData.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <MVRCore/CameraOffAxis.H>
#include "app/include/Axis.h"
#include "app/include/Feedback.h"
#include "app/include/ExperimentMgr.h"
#include "app/include/CurrentHCIMgr.h"

class App : public MinVR::AbstractMVRApp {
public:
	App();
	virtual ~App();

	void doUserInputAndPreDrawComputation(const std::vector<MinVR::EventRef> &events, double synchronizedTime);
	void initializeContextSpecificVars(int threadId, MinVR::WindowRef window);
	void postInitialization();
	void drawGraphics(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
	glm::dvec3 convertScreenToRoomCoordinates(glm::dvec2 screenCoords);

private:
	void initGL();
	void initVBO(int threadId, MinVR::WindowRef window);
	void initLights();
	std::shared_ptr<GPUMesh> cubeMesh;
	std::shared_ptr<GPUMesh> tetraMesh;
	std::shared_ptr<GPUMesh> axisMesh;
	std::shared_ptr<GPUMesh> sphereMesh;
	std::shared_ptr<GPUMesh> quadMesh;
	GPUMeshRef bgMesh;
	std::shared_ptr<GLSLProgram> shader;
	std::shared_ptr<GLSLProgram> bgShader;
	std::map<int, GLuint> _vboId;

	CurrentHCIMgrRef currentHCIMgr;
	std::shared_ptr<CFrameMgr> cFrameMgr;
	TextureMgrRef texMan; 
	AxisRef axis;
	std::shared_ptr<MinVR::CameraOffAxis> offAxisCamera;
	FeedbackRef feedback;
	ExperimentMgrRef experimentMgr;

	AbstractHCIRef newYTransHCI;
	AbstractHCIRef newXZRotHCI;
	AbstractHCIRef newAnchoredHCI;
	AbstractHCIRef oldYTransHCI;
	AbstractHCIRef oldXZRotHCI;
	AbstractHCIRef oldAnchoredHCI;

};

#endif /* APP_H_ */
