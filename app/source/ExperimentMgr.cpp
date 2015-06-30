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

	trialNumber = 1; // 1 - 5 
	newAnchored = false; // initialization depends on config file. don't set to false.
	transformIndex = 1; // 1 - 5 but randomized

	// get transforms
	transMat1 = MinVR::ConfigVal("TransMats1", glm::dmat4(1.0), false); // identity so far
	experimentNumber = 0; // initialization depends on config file.


	double d = MinVR::ConfigVal("Test", 0.5, false);
	std::vector<std::string> strings = MinVR::splitStringIntoArray(MinVR::ConfigVal("TestMulti", "", false));

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
	
	currentHCIMgr->currentHCI.reset(new NewAnchoredExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback)); //call whatever 

	currentHCIMgr->currentHCI->initializeContextSpecificVars(threadId, window);
	std::cout<<" DONE resting current HCI from ExperimentMgr"<<std::endl;
}


// each trial is a tetra's orientation specified by a dmat4, in a config file
// and a restart in time
// and a new file to log output
// and a switch in the current HCI
void ExperimentMgr::switchTrial () {

}

// Does the following:
// switch HCI
// update trial number
// update experiment number
// point to next matrices we need for experiment
void ExperimentMgr::advance () {

}

glm::dmat4 ExperimentMgr::getTransforms(){
	
/*fill in later*/	return glm::dmat4(0.0);

}

// assume Cframe manager has updated matrices
// since App calls currentHCI->update before this call
bool ExperimentMgr::checkFinish() {

	const double nearEnough = 0.03;
	
	glm::dmat4 currHCItransform = cFrameMgr->getVirtualToRoomSpaceFrame();
	glm::dvec3 transformableTetraPointA = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointA, 1.0));
	glm::dvec3 staticTetraPointA = glm::dvec3(glm::translate(glm::dmat4(1.0),glm::dvec3(-0.9, 0.0, 0.0)) * glm::dvec4(tetra->pointA, 1.0));

	glm::dvec3 transformableTetraPointB = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointA, 1.0));
	glm::dvec3 staticTetraPointB = glm::dvec3(glm::translate(glm::dmat4(1.0),glm::dvec3(-0.9, 0.0, 0.0)) * glm::dvec4(tetra->pointA, 1.0));

	glm::dvec3 transformableTetraPointC = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointA, 1.0));
	glm::dvec3 staticTetraPointC = glm::dvec3(glm::translate(glm::dmat4(1.0),glm::dvec3(-0.9, 0.0, 0.0)) * glm::dvec4(tetra->pointA, 1.0));

	glm::dvec3 transformableTetraPointD = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointA, 1.0));
	glm::dvec3 staticTetraPointD = glm::dvec3(glm::translate(glm::dmat4(1.0),glm::dvec3(-0.9, 0.0, 0.0)) * glm::dvec4(tetra->pointA, 1.0));

	/*std::cout << "Static point: " << glm::to_string(staticTetraPointA) << std::endl;
	std::cout << "Transformable point: " << glm::to_string(transformableTetraPointA) << std::endl;
	std::cout << "Distance: " << glm::distance(transformableTetraPointA, staticTetraPointA) << std::endl;*/

	bool nearA = glm::distance(transformableTetraPointA, staticTetraPointA) < nearEnough;
	bool nearB = glm::distance(transformableTetraPointB, staticTetraPointB) < nearEnough;
	bool nearC = glm::distance(transformableTetraPointC, staticTetraPointC) < nearEnough;
	bool nearD = glm::distance(transformableTetraPointD, staticTetraPointD) < nearEnough;

	if (nearA && nearB && nearC && nearD) {
		//std::cout << "You are winner. Ha ha ha!" << std::endl;
		return true;
	}
	
	//std::cout << "Y U NAGUT WAINNING?" << std::endl;
	return false;

}

void ExperimentMgr::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window) {
	// use transforms stored in the std::vector
	// apply them to whatever objects we're rendering, and draw them.

	camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), glm::dvec3(-0.5, 0.0, 1.0)));
	//shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());

	tetra->draw(threadId, camera, window, "Koala2");
}




