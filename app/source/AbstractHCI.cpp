#include "app/include/AbstractHCI.h"

AbstractHCI::AbstractHCI(CFrameMgrRef mgr, FeedbackRef feedback) {
	this->cFrameMgr = mgr;
	this->feedback = feedback;
}

AbstractHCI::~AbstractHCI(){

}