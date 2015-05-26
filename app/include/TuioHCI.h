#ifndef TUIOHCI_H_
#define TUIOHCI_H_

#include "AbstractHCI.h"
#include "MVRCore/AbstractCamera.H"

class TuioHCI : public AbstractHCI {

public:
	TuioHCI();
	virtual ~TuioHCI();
	void update(std::vector<MinVR::Event> events) ;
    void draw(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
private:
	CFrameMgrRef CFrameRef;

};

#endif /* TUIOHCI_H_ */