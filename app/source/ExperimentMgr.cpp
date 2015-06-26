/* 

Need to know how to switch the currentHCI with another HCI
1. Transition betw trials
2. Transition betw HCIs
3. Record data
4. 

*/

#include "ExperimentMgr.h"

ExperimentMgr::ExperimentMgr(AbstractHCIRef &currentHCI) {
	this->currentHCI = currentHCI
	
}

ExperimentMgr::~ExperimentMgr() {
	
}


