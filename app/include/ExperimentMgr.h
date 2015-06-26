#ifndef EXPERIMENTMGR_H_
#define EXPERIMENTMGR_H_

// what things do we need to include anyway?
// #include "GL/glew.h"


class ExperimentMgr {
public:
	ExperimentMgr(AbstractHCIRef &currentHCI);
	virtual ~ExperimentMgr();


private:
	void initGL();
	void initVBO(int threadId, MinVR::WindowRef window);
	AbstractHCIRef currentHCI;
	TetrahedronRef tetra;
};

#endif /* EXPERIMENTMGR_H_ */
