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

	likertCount = 0;
	
	//numberQuestions = MinVR::ConfigVal("NumberQuestions", 1, false); 
	showCompleteTrial = false;
	secondTimer = false;
	inPosition = false;
	// ExperimentMgr doesn't need texture manager, but tetrahedron does.
	tetra.reset(new Tetrahedron(window->getCamera(0), cFrameMgr, texMan, nearEnough));
	tetra->initializeContextSpecificVars(threadId);

	//////////////////////////
	// Experiment Variables //
	//////////////////////////
	trialCount = 0; // 0 - 4
	trialSet = 1; // 1 - 2, for updating HCIExperiment number
	HCIExperiment = 0; // 0 - 6
	newAnchored = MinVR::ConfigVal("newAnchored", false, false); 
	transformIndex = 1; // 1 - 5 but randomized
	numTrials = MinVR::ConfigVal("NumTrials", 5, false);
	numPracticeTrials = MinVR::ConfigVal("NumPracticeTrials", 3, false);

	/*
	if(HCIExperiment == 0) {
		currentHCIMgr->currentHCI.reset(new LikertHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 1) {
		currentHCIMgr->currentHCI.reset(new NewYTransExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 2) {
		currentHCIMgr->currentHCI.reset(new NewXZRotExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 3) {
		currentHCIMgr->currentHCI.reset(new NewAnchoredExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 4) {
		currentHCIMgr->currentHCI.reset(new OrigYTransExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 5) {
		currentHCIMgr->currentHCI.reset(new OrigXZRotExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	if(HCIExperiment == 6) {
		currentHCIMgr->currentHCI.reset(new OrigAnchoredHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	}
	
	currentHCIMgr->currentHCI->initializeContextSpecificVars(threadId, window);
	*/

	// get translation transforms
	glm::dmat4 transMat1 = MinVR::ConfigVal("TransMat1", glm::dmat4(0.0), false); 
	glm::dmat4 transMat2 = MinVR::ConfigVal("TransMat2", glm::dmat4(0.0), false); 
	glm::dmat4 transMat3 = MinVR::ConfigVal("TransMat3", glm::dmat4(0.0), false); 
	glm::dmat4 transMat4 = MinVR::ConfigVal("TransMat4", glm::dmat4(0.0), false); 
	glm::dmat4 transMat5 = MinVR::ConfigVal("TransMat5", glm::dmat4(0.0), false);
	glm::dmat4 transMat6 = MinVR::ConfigVal("TransMat6", glm::dmat4(0.0), false); 
	glm::dmat4 transMat7 = MinVR::ConfigVal("TransMat7", glm::dmat4(0.0), false);
	glm::dmat4 transMat8 = MinVR::ConfigVal("TransMat8", glm::dmat4(0.0), false);

	// put transforms into a vector
	transMats.push_back(transMat1);
	transMats.push_back(transMat2);
	transMats.push_back(transMat3);
	transMats.push_back(transMat4);
	transMats.push_back(transMat5);
	transMats.push_back(transMat6);
	transMats.push_back(transMat7);
	transMats.push_back(transMat8);

	//// get rotation transforms
	glm::dmat4 rotMat1 = MinVR::ConfigVal("RotMat1", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat2 = MinVR::ConfigVal("RotMat2", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat3 = MinVR::ConfigVal("RotMat3", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat4 = MinVR::ConfigVal("RotMat4", glm::dmat4(0.0), false); 
	glm::dmat4 rotMat5 = MinVR::ConfigVal("RotMat5", glm::dmat4(0.0), false);
	glm::dmat4 rotMat6 = MinVR::ConfigVal("RotMat6", glm::dmat4(0.0), false);
	glm::dmat4 rotMat7 = MinVR::ConfigVal("RotMat7", glm::dmat4(0.0), false);
	glm::dmat4 rotMat8 = MinVR::ConfigVal("RotMat8", glm::dmat4(0.0), false);

	// put transforms into a vector
	rotMats.push_back(rotMat1);
	rotMats.push_back(rotMat2);
	rotMats.push_back(rotMat3);
	rotMats.push_back(rotMat4);
	rotMats.push_back(rotMat5);
	rotMats.push_back(rotMat6);
	rotMats.push_back(rotMat7);
	rotMats.push_back(rotMat8);

	//// get combined transforms
	glm::dmat4 combinedMat1 = MinVR::ConfigVal("CombinedMat1", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat2 = MinVR::ConfigVal("CombinedMat2", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat3 = MinVR::ConfigVal("CombinedMat3", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat4 = MinVR::ConfigVal("CombinedMat4", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat5 = MinVR::ConfigVal("CombinedMat5", glm::dmat4(0.0), false);
	glm::dmat4 combinedMat6 = MinVR::ConfigVal("CombinedMat6", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat7 = MinVR::ConfigVal("CombinedMat7", glm::dmat4(0.0), false); 
	glm::dmat4 combinedMat8 = MinVR::ConfigVal("CombinedMat8", glm::dmat4(0.0), false); 

	// put transforms into a vector
	combinedMats.push_back(combinedMat1);
	combinedMats.push_back(combinedMat2);
	combinedMats.push_back(combinedMat3);
	combinedMats.push_back(combinedMat4);
	combinedMats.push_back(combinedMat5);
	combinedMats.push_back(combinedMat6);
	combinedMats.push_back(combinedMat7);
	combinedMats.push_back(combinedMat8);

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
void ExperimentMgr::advance (bool newOld) {

	if(HCIExperiment != 0){
		double diff = (getDuration(trialStart, trialEnd)).total_milliseconds();
		std::cout<<"Total Time In Trial"<<diff<<std::endl;
		std::cout<< "The start time was: " << trialStart << std::endl;
		std::cout<< "The end time was  : " << trialEnd << std::endl;
		trialStart = getCurrentTime();
	}



	if(newOld){
		//trialSet = 1;
	    //likertCount = 0;
		if (trialSet == 4 ) {
		std::cout << "Finished :D" << std::endl;
		}
		trialCount += 1;
		if(trialCount == numTrials){ //finish a set of trials
			trialCount = 0;		
			likertCount += 1;
			HCIExperiment = 0;
			if((likertCount % 2) == 0){
				trialSet += 1;
			}
			
		}
		else if(HCIExperiment == 0){ //in Likertmode
			trialCount -= 1;
			if((likertCount % 2) != 0){
				HCIExperiment = likertCount+ 4 - trialSet ;
			}
			else{
				//insert promp here in future
				HCIExperiment = trialSet;
			}
		}  
	}

	else{ //oldNew case
		
		if (trialSet == 4 ) {
		std::cout << "Finished :D" << std::endl;
		}
		
		trialCount += 1;
		if(trialCount == numTrials){ //finish a set of trials
			trialCount = 0;		
			likertCount += 1;
			HCIExperiment = 0;
			if((likertCount % 2) == 0){
				trialSet += 1;
			}
			
		}
		else if(HCIExperiment == 0){ //in Likertmode
			trialCount -= 1;
			if((likertCount % 2) != 0){
				HCIExperiment =  trialSet ;
			}
			else{
				//insert promp here in future
				HCIExperiment = likertCount+ 5 -trialSet;
			}
		}  
	
	}
	
	
	if(HCIExperiment != 0 && trialCount < numPracticeTrials) {
		feedback->displayPractice = true;
	}
	else {
		feedback->displayPractice = false;
	}
	
	std::cout << "trial count: " << trialCount << std::endl;
	std::cout << "trial set: " << trialSet << std::endl;
	std::cout << "likertCount: " << likertCount << std::endl;
	std::cout << "experiment number: " << HCIExperiment << std::endl;

	
}



void ExperimentMgr::resetTimer(){

}


// assume Cframe manager has updated matrices
// since App calls currentHCI->update before this call
bool ExperimentMgr::checkFinish() {


	glm::dmat4 currHCItransform = cFrameMgr->getVirtualToRoomSpaceFrame();

	if (HCIExperiment == 0) {
 
		LikertHCI* likert = dynamic_cast<LikertHCI*>((currentHCIMgr->currentHCI).get());
		if (likert->done) {
			likert->done = false;
			trialStart = getCurrentTime();
			return true;
		};
	}
	else if (HCIExperiment == 1) {
		transform = transMats[trialCount];
	} else if (HCIExperiment == 2) {
		transform = rotMats[trialCount];
	} else if (HCIExperiment == 3) {
		transform = combinedMats[trialCount];
	}
	else if (HCIExperiment == 4) {
		transform = transMats[trialCount];
	} else if (HCIExperiment == 5) {
		transform = rotMats[trialCount];
	} else if (HCIExperiment == 6) {
		transform = combinedMats[trialCount];
	}
	
	
	//std::cout<<"staticTransform: "<<glm::to_string(staticTransform)<<std::endl;
	
	/*std::cout << "Muh xforms: " << glm::to_string(transMats[0]) << std::endl;
	std::cout << "Muh xforms2: " << glm::to_string(transMats[1]) << std::endl;
	std::cout << "Muh xforms3: " << glm::to_string(transMats[2]) << std::endl;
	std::cout << "Muh xforms4: " << glm::to_string(transMats[3]) << std::endl;
	std::cout << "Muh xforms5: " << glm::to_string(transMats[4]) << std::endl;*/

	// points in Model space are identity transformed to World to Room
	glm::dvec3 transformableTetraPointA = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointA, 1.0));
	glm::dvec3 staticTetraPointA = tetra->pointA;

	glm::dvec3 transformableTetraPointB = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointB, 1.0));
	glm::dvec3 staticTetraPointB = tetra->pointB;

	glm::dvec3 transformableTetraPointC = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointC, 1.0));
	glm::dvec3 staticTetraPointC = tetra->pointC;

	glm::dvec3 transformableTetraPointD = glm::dvec3(currHCItransform * transform * glm::dvec4(tetra->pointD, 1.0));
	glm::dvec3 staticTetraPointD = tetra->pointD;

	//std::cout << "Static point: " << glm::to_string(staticTetraPointA) << std::endl;
	//std::cout << "Transformable point: " << glm::to_string(transformableTetraPointA) << std::endl;
	//std::cout << "Distance: " << glm::distance(transformableTetraPointA, staticTetraPointA) << std::endl;

	
	bool nearA = glm::distance(transformableTetraPointA, staticTetraPointA) < nearEnough;
	bool nearB = glm::distance(transformableTetraPointB, staticTetraPointB) < nearEnough;
	bool nearC = glm::distance(transformableTetraPointC, staticTetraPointC) < nearEnough;
	bool nearD = glm::distance(transformableTetraPointD, staticTetraPointD) < nearEnough;
	bool prevInPosition = inPosition;

	if (nearA && nearB && nearC && nearD && HCIExperiment != 0){ //if in the correct posisition
		
		inPosition = true;
		showCompleteTrial = true;		
	}
	else{
	
		inPosition = false;
		secondTimer = false;
		showCompleteTrial = false;	
	}

	//std::cout<<"inPosition : "<<inPosition<<std::endl;
	//std::cout<<"prevInPosition : "<<prevInPosition<<std::endl;

	if(inPosition == true /*to guard the situation when go out of time stamp */ && inPosition != prevInPosition /*make sure only start the time stamp initially*/){
		startInZone = getCurrentTime();
		secondTimer = true;

		
	}

	

	if(secondTimer){
		
		t2 = getCurrentTime();
		totalTimeInZone = (getDuration(t2, startInZone)).total_milliseconds();
	
	}

	
	
	//std::cout<<"total time in zone :  " <<totalTimeInZone<<std::endl;

	if (nearA && nearB && nearC && nearD &&  totalTimeInZone > 1000.0 ) {
		
		// set to false for next trial
		showCompleteTrial = false;
		trialEnd = getCurrentTime();

		return true;
	}
	
	return false;

	//MinVR::TimeStamp timeStamp = getCurrentTime();
	//MinVR::TimeStamp t2 = getCurrentTime();
	//double diff = (getDuration(timeStamp, t2)).total_milliseconds();

}


float ExperimentMgr::getError(float A, float B, float C, float D){

	return (A+B+C+D)/4;
}


void ExperimentMgr::draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window) {
	// use transforms stored in the std::vector
	// apply them to whatever objects we're rendering, and draw them.

	if (HCIExperiment != 0 && showCompleteTrial == false) {
	
		// draws both the static and the transformable tetrahedron
		tetra->draw(threadId, camera, window, "Koala2", transform, "red",  "green", "blue", "Koala", "forestGreen");
	} 
	else if(HCIExperiment != 0 && showCompleteTrial == true){
	
		tetra->draw(threadId, camera, window, "forestGreen", transform, "forestGreen",  "forestGreen", "forestGreen", "forestGreen", "forestGreen");
	} 
	else { // this is the likertHCI
		currentHCIMgr->currentHCI->draw(threadId, camera, window);
	}
}




