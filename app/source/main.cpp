#include "AppKit_GLFW/MVREngineGLFW.H"
#include "MVRCore/DataFileUtils.H"
#include <iostream>
#include "app/include/App.h"

int main(int argc, char** argv)
{
	MinVR::DataFileUtils::addFileSearchPath("$(MinVR_DIR)/share/vrsetup");
	MinVR::DataFileUtils::addFileSearchPath("$(MinVR_DIR)/share/shaders");
	MinVR::AbstractMVREngine *engine = new MinVR::MVREngineGLFW();
	engine->init(argc, argv);
	MinVR::AbstractMVRAppRef app(new App());
	engine->runApp(app);
	delete engine;

	return 0;
}
