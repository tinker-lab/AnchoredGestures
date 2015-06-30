/* 

Need to know how to switch the currentHCI with another HCI
1. Transition betw trials
2. Transition betw HCIs
3. Record data
4. 

*/

#include "app/include/ExperimentMgr.h"


ExperimentMgr::ExperimentMgr(CurrentHCIMgrRef currentHCIMgr, CFrameMgrRef mgr, MinVR::AbstractCameraRef camera, TextureMgrRef texMan, FeedbackRef feedback) { //might need ampersand
	
	this->cFrameMgr = mgr;
	this->currentHCIMgr = currentHCIMgr; // some new HCIs
	this->texMan = texMan;
	this->feedback = feedback;
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	trialNumber = 0;
	newAnchored = false; // initialization depends on config file. don't set to false.
	transformIndex = 0;
	//transforms = getTransforms(); // don't know how this works 
	experimentNumber = 0; // initialization depends on config file.
}	

ExperimentMgr::~ExperimentMgr() {
	
}

// apparently you're going to have a bunch of dmat4s loaded into a vector...
void ExperimentMgr::switchHCI () {

}

void ExperimentMgr::initializeContextSpecificVars(int threadId, MinVR::WindowRef window){

	// ExperimentMgr doesn't need texture manager, but tetrahedron does.
	tetra.reset(new Tetrahedron(window->getCamera(0), cFrameMgr, texMan));
	tetra->initializeContextSpecificVars(threadId);
	std::cout<<"resting current HCI from ExperimentMgr"<<std::endl;
	
	currentHCIMgr->currentHCI.reset(new NewXZRotExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback)); //call whatever 

	currentHCIMgr->currentHCI->initializeContextSpecificVars(threadId, window);
	std::cout<<" DONE resting current HCI from ExperimentMgr"<<std::endl;
}


// each trial is a tetra's orientation specified by a dmat4, in a config file
// and a restart in time
// and a new file to log output
// and a switch in the current HCI
void ExperimentMgr::switchTrial () {

}

// calls a bunch of other methods and advances things as a whole
void ExperimentMgr::advance () {

}

glm::dmat4 ExperimentMgr::getTransforms(){
	
/*fill in later*/	return glm::dmat4(0.0);

}

void ExperimentMgr::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window) {
	// use transforms stored in the std::vector
	// apply them to whatever objects we're rendering, and draw them.

	camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), glm::dvec3(-0.5, 0.0, 1.0)));
	//shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());

	tetra->draw(threadId, camera, window, "Koala2");
}




