/* 

Need to know how to switch the currentHCI with another HCI
1. Transition betw trials
2. Transition betw HCIs
3. Record data
4. 

*/

#include "ExperimentMgr.h"

ExperimentMgr::ExperimentMgr(AbstractHCIRef currentHCI) { //might need ampersand
	
	this->currentHCI = currentHCI; // some new HCIs
	trialNumber = 0;
	newAnchored = false; // initialization depends on config file. don't set to false.
	transformIndex = 0;
	transforms = getTransforms(); // don't know how this works 
	experimentNumber = 0; // initialization depends on config file.
}

ExperimentMgr::~ExperimentMgr() {
	
}

// apparently you're going to have a bunch of dmat4s loaded into a vector...
void ExperimentMgr::switchHCI () {

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




