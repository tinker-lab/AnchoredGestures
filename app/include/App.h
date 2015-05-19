#ifndef APP_H_
#define APP_H_

#include "GL/glew.h"
#include "MVRCore/AbstractMVRApp.H"
#include "MVRCore/AbstractCamera.H"
#include "MVRCore/AbstractWindow.H"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MVRCore/Event.H"
#include <GLFW/glfw3.h>
#include <vector>
#include <map>

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

	std::map<int, GLuint> _vboId;
};

#endif /* APP_H_ */
