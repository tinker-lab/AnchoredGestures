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
#include "MVRCore/Event.H"
#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include "app/include/GPUMesh.h"
#include "app/include/GLSLProgram.h"
#include "app/include/TuioHCI.h"
#include "app/include/AbstractHCI.h"
#include "app/include/TextureMgr.h"

class App : public MinVR::AbstractMVRApp {
public:
	App();
	virtual ~App();

	void doUserInputAndPreDrawComputation(const std::vector<MinVR::EventRef> &events, double synchronizedTime);
	void initializeContextSpecificVars(int threadId, MinVR::WindowRef window);
	void postInitialization();
	void drawGraphics(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);

private:
	void initGL();
	void initVBO(int threadId);
	void initLights();
	std::shared_ptr<GPUMesh> cubeMesh;
	std::shared_ptr<GLSLProgram> shader;
	std::map<int, GLuint> _vboId;
	std::shared_ptr<AbstractHCI> currentHCI;
	std::shared_ptr<CFrameMgr> cFrameMgr;
	TextureMgrRef texMan; 
};

#endif /* APP_H_ */
