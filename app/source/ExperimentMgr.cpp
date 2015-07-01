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
	startTime = getCurrentTime();
}	

ExperimentMgr::~ExperimentMgr() {
	
}




void ExperimentMgr::initializeContextSpecificVars(int threadId, MinVR::WindowRef window){

	// ExperimentMgr doesn't need texture manager, but tetrahedron does.
	tetra.reset(new Tetrahedron(window->getCamera(0), cFrameMgr, texMan));
	tetra->initializeContextSpecificVars(threadId);
	std::cout<<"resting current HCI from ExperimentMgr"<<std::endl;
	
	std::cout<<" DONE resting current HCI from ExperimentMgr"<<std::endl;

	//////////////////////////
	// Experiment Variables //
	//////////////////////////
	trialCount = 0; // 1 - 5 
	HCIExperiment = 1;
	newAnchored = false; // initialization depends on config file. don't set to false.
	transformIndex = 1; // 1 - 5 but randomized


	if(HCIExperiment == 1){
		currentHCIMgr->currentHCI.reset(new NewYTransExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 2){
		currentHCIMgr->currentHCI.reset(new NewXZRotExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 3){
		currentHCIMgr->currentHCI.reset(new NewAnchoredExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	
	currentHCIMgr->currentHCI->initializeContextSpecificVars(threadId, window);


	// get translation transforms
	glm::dmat4 transMat1 = MinVR::ConfigVal("TransMat1", glm::dmat4(0.0), false); 
	glm::dmat4 transMat2 = MinVR::ConfigVal("TransMat2", glm::dmat4(0.0), false); 
	glm::dmat4 transMat3 = MinVR::ConfigVal("TransMat3", glm::dmat4(0.0), false); 
	glm::dmat4 transMat4 = MinVR::ConfigVal("TransMat4", glm::dmat4(0.0), false); 
	glm::dmat4 transMat5 = MinVR::ConfigVal("TransMat5", glm::dmat4(0.0), false); 

	// put transforms into a vector
	transMats.push_back(transMat1);
	transMats.push_back(transMat2);
	transMats.push_back(transMat3);
	transMats.push_back(transMat4);
	transMats.push_back(transMat5);

	//// get rotation transforms
	glm::dmat4 rotMat1 = MinVR::ConfigVal("RotMat1", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat2 = MinVR::ConfigVal("RotMat2", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat3 = MinVR::ConfigVal("RotMat3", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat4 = MinVR::ConfigVal("RotMat4", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat5 = MinVR::ConfigVal("RotMat5", glm::dmat4(0.0), false); 

	// put transforms into a vector
	rotMats.push_back(rotMat1);
	rotMats.push_back(rotMat2);
	rotMats.push_back(rotMat3);
	rotMats.push_back(rotMat4);
	rotMats.push_back(rotMat5);

	//// get combined transforms
	glm::dmat4 combinedMat1 = MinVR::ConfigVal("CombinedMat1", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat2 = MinVR::ConfigVal("CombinedMat2", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat3 = MinVR::ConfigVal("CombinedMat3", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat4 = MinVR::ConfigVal("CombinedMat4", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat5 = MinVR::ConfigVal("CombinedMat5", glm::dmat4(0.0), false); 

	// put transforms into a vector
	combinedMats.push_back(combinedMat1);
	combinedMats.push_back(combinedMat2);
	combinedMats.push_back(combinedMat3);
	combinedMats.push_back(combinedMat4);
	combinedMats.push_back(combinedMat5);

 // initialization depends on config file.
	double d = MinVR::ConfigVal("Test", 0.5, false);
	std::vector<std::string> strings = MinVR::splitStringIntoArray(MinVR::ConfigVal("TestMulti", "", false));
}


// each trial is a tetra's orientation specified by a dmat4, in a config file
// and a restart in time
// and a new file to log output


// Does the following:
// switch HCI
// update trial number
// update experiment number
// point to next matrices we need for experiment
void ExperimentMgr::advance () {
	trialCount += 1;
	if(trialCount == 5){
		trialCount = 0;
		//swtichHCI becauuse finish all trials 
	}
	
}


void ExperimentMgr::resetTimer(){

}

glm::dmat4 ExperimentMgr::getTransforms(){
	
/*fill in later*/	return glm::dmat4(0.0);

}


// apparently you're going to have a bunch of dmat4s loaded into a vector...
void ExperimentMgr::switchHCI () {
	
}


// assume Cframe manager has updated matrices
// since App calls currentHCI->update before this call
bool ExperimentMgr::checkFinish() {

	const double nearEnough = 0.03;
	
	glm::dmat4 currHCItransform = cFrameMgr->getVirtualToRoomSpaceFrame();
	staticTransform = transMats[trialCount]; //glm::translate(glm::dmat4(1.0),glm::dvec3(-0.9, 0.0, 0.0));
	
	std::cout<<"staticTransform: "<<glm::to_string(staticTransform)<<std::endl;
	
	/*std::cout << "Muh xforms: " << glm::to_string(transMats[0]) << std::endl;
	std::cout << "Muh xforms2: " << glm::to_string(transMats[1]) << std::endl;
	std::cout << "Muh xforms3: " << glm::to_string(transMats[2]) << std::endl;
	std::cout << "Muh xforms4: " << glm::to_string(transMats[3]) << std::endl;
	std::cout << "Muh xforms5: " << glm::to_string(transMats[4]) << std::endl;*/

	glm::dvec3 transformableTetraPointA = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointA, 1.0));
	glm::dvec3 staticTetraPointA = glm::dvec3(staticTransform * glm::dvec4(tetra->pointA, 1.0));

	glm::dvec3 transformableTetraPointB = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointB, 1.0));
	glm::dvec3 staticTetraPointB = glm::dvec3(staticTransform * glm::dvec4(tetra->pointB, 1.0));

	glm::dvec3 transformableTetraPointC = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointC, 1.0));
	glm::dvec3 staticTetraPointC = glm::dvec3(staticTransform * glm::dvec4(tetra->pointC, 1.0));

	glm::dvec3 transformableTetraPointD = glm::dvec3(currHCItransform * glm::dvec4(tetra->pointD, 1.0));
	glm::dvec3 staticTetraPointD = glm::dvec3(staticTransform * glm::dvec4(tetra->pointD, 1.0));

	/*std::cout << "Static point: " << glm::to_string(staticTetraPointA) << std::endl;
	std::cout << "Transformable point: " << glm::to_string(transformableTetraPointA) << std::endl;
	std::cout << "Distance: " << glm::distance(transformableTetraPointA, staticTetraPointA) << std::endl;*/

	bool nearA = glm::distance(transformableTetraPointA, staticTetraPointA) < nearEnough;
	bool nearB = glm::distance(transformableTetraPointB, staticTetraPointB) < nearEnough;
	bool nearC = glm::distance(transformableTetraPointC, staticTetraPointC) < nearEnough;
	bool nearD = glm::distance(transformableTetraPointD, staticTetraPointD) < nearEnough;

	if (nearA && nearB && nearC && nearD) {
		std::cout<<"ayyyyyyyy gurlll"<<std::endl;
		return true; 
	}
	
	return false;

}

void ExperimentMgr::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window) {
	// use transforms stored in the std::vector
	// apply them to whatever objects we're rendering, and draw them.

	camera->setObjectToWorldMatrix(glm::translate(glm::dmat4(1.0f), glm::dvec3(-0.5, 0.0, 1.0)));
	
	// draws both the static and the transformable tetrahedron
	tetra->draw(threadId, camera, window, "Koala2", staticTransform );
}




