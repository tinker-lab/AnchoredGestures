#ifndef ABSTRACTHCI_H_
#define ABSTRACTHCI_H_

#include <memory>
#include <glm/glm.hpp>
#include "CFrameMgr.H"
#include <vector>
#include "MVRCore/Event.H"
 

typedef std::shared_ptr<class AbstractHCI> AbstractHCIRef;

class AbstractHCI
{
public:
	AbstractHCI() {}
	virtual ~AbstractHCI() {}
	virtual void update(std::vector<MinVR::Event> events) = 0;
	virtual void draw() = 0;
protected:
	CFrameMgrRef CFrameRef;
	
};


#endif /* ABSTRACTHCI_H_ */