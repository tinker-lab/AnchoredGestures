
#include "OrigAnchoredHCI.h"
/*
OrigAnchoredHCI::OrigAnchoredHCI(MinVR::AbstractCameraRef camera, CFrameMgrRef cFrameMgr, TextureMgrRef texMan) : AbstractHCI(cFrameMgr)
{
	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(camera);
	this->texMan = texMan;

	_touch1IsValid = false;
	_touch2IsValid = false;
	_rotating = false;
	_lastRotationAxis = glm::dvec3(1,0,0);
		
}

OrigAnchoredHCI::~OrigAnchoredHCI()
{
}

bool OrigAnchoredHCI::offerTouchDown(MinVR::EventRef event)
{
	glm::dvec3 roomCoord = convertScreenToRoomCoordinates(event->get2DData());
	TouchDataRef info(new TouchData(event, roomCoord));
	//info.id = id;
	//info.screenPos = pos;
	//info.lastScreenPos = pos;
	//info.pos = convertScreenToRoomCoordinates(pos);
	//info.lastPos = convertScreenToRoomCoordinates(pos);
	//info.isValid = true;
	
	// Try to remove touches from leaning on the table
	//TODO update these numbers with the new screen size
	if (abs(info->getCurrRoomPos().x) > 1.75 || abs(info->getCurrRoomPos().z) > 1.25) {
		return false;
	}
	
	if (!_touch1IsValid) {
		_touch1 = info;
		determineTouchToHandCoorespondence(_touch1);
		if (_touch2IsValid) {
			_centerAxis = _touch1->getCurrRoomPos() + (0.5*(_touch2->getCurrRoomPos()-_touch1->getCurrRoomPos()));
			_feedbackWidget->setFingerIndicatorPositions(_touch1.pos, _touch2.pos);
		}
		//cout << "Touch1 down"<<endl;
		return true;
	}
	else if (!_touch2IsValid) {
		_touch2 = info;
		determineTouchToHandCoorespondence(_touch2);
		if(_touch1IsValid) {
			_centerAxis = _touch1->getCurrRoomPos() + (0.5*(_touch2->getCurrRoomPos()-_touch1->getCurrRoomPos()));
			_feedbackWidget->setFingerIndicatorPositions(_touch1.pos, _touch2.pos);
		}
		//cout << "Touch2 down"<<endl;
		return true;
	}

	return false;
}

bool OrigAnchoredHCI::offerTouchUp(int id)
{
	if (_touch1IsValid && _touch1.id == id) {
		_touch1.isValid = false;
		_roomToWidgetHemisphere = CoordinateFrame();
		_feedbackWidget->resetGlobe();
		_touch1Moves.clear();
		_leftAxisHistory.clear();
		_rightAxisHistory.clear();
		//cout << "Touch1 up"<<endl;
		return true;
	}
	else if (_touch2.isValid && _touch2.id == id) {
		_touch2.isValid = false;
		_roomToWidgetHemisphere = CoordinateFrame();
		_feedbackWidget->resetGlobe();
		_touch2Moves.clear();
		_leftAxisHistory.clear();
		_rightAxisHistory.clear();
		//cout << "Touch2 up"<<endl;
		return true;
	}

	return false;	
}

bool TouchAnchoredRotHCI::offerTouchMove(int id, Vector2 pos)
{
	bool axisChanged = false;
	if (_touch1.isValid && id == _touch1.id) {
		if (_touch1.lastPos != _touch1.pos) {
			_touch1.lastPos = _touch1.pos;
		}
		_touch1.pos = convertScreenToRoomCoordinates(pos);
		_touch1.lastScreenPos = _touch1.screenPos;
		_touch1.screenPos = pos;
		movement move;
		move.touchName = "touch1";
		move.distance = (_touch1.screenPos - _touch1.lastScreenPos).length();
		timeval curTime;
		gettimeofday(&curTime,NULL);
		move.timeStamp = curTime;
		_touch1Moves.append(move);
		
		if(_touch2.isValid) {
			axisChanged = true;
			alignXform(_touch1.lastPos, _touch2.pos, _touch1.pos);
		}
		else {
			// just translate
			CoordinateFrame trans(_touch1.lastPos - _touch1.pos);
			CoordinateFrame newFrame = _gfxMgr->getRoomToVirtualSpaceFrame() * trans;
			if (!testForManipulationOutsideOfBounds(newFrame)&&!testForCrazyManipulation(trans)) {
				_gfxMgr->setRoomToVirtualSpaceFrame(newFrame);
				_totalTranslationLength+=trans.translation.length();
			}
		}
	}
	else if (_touch2.isValid && id == _touch2.id) {
		if (_touch2.lastPos != _touch2.pos) {
			_touch2.lastPos = _touch2.pos;
		}
		_touch2.pos = convertScreenToRoomCoordinates(pos);
		_touch2.lastScreenPos = _touch2.screenPos;
		_touch2.screenPos = pos;
		movement move;
		move.touchName = "touch2";
		move.distance = (_touch2.screenPos - _touch2.lastScreenPos).length();
		timeval curTime;
		gettimeofday(&curTime,NULL);
		move.timeStamp = curTime;
		_touch2Moves.append(move);
		if(_touch1.isValid) {
			axisChanged = true;
			alignXform(_touch2.lastPos, _touch1.pos, _touch2.pos);
		}
		else {
			// just translate
			CoordinateFrame trans(_touch2.lastPos - _touch2.pos);
			CoordinateFrame newFrame = _gfxMgr->getRoomToVirtualSpaceFrame() * trans;
			if (!testForManipulationOutsideOfBounds(newFrame)&&!testForCrazyManipulation(trans)) {
				_gfxMgr->setRoomToVirtualSpaceFrame(newFrame);
				_totalTranslationLength+=trans.translation.length();
			}
		}
	}
	if (axisChanged) {
		_centerAxis = _touch1.pos + (0.5*(_touch2.pos-_touch1.pos));
		_feedbackWidget->setFingerIndicatorPositions(_touch1.pos, _touch2.pos);
	}
}

void TouchAnchoredRotHCI::alignXform(Vector3 src1, Vector3 src2, Vector3 dst1)
{
	double scale = (src2 - dst1).length() / (src2 - src1).length();
	
	CoordinateFrame scalemat(Matrix3(scale, 0, 0,  0, scale, 0,  0, 0, scale), Vector3::zero());
	
	CoordinateFrame targetXform = CoordinateFrame(dst1) * scalemat  * CoordinateFrame(-dst1);
	
	Vector3 initial = (src1-src2).unit();
	Vector3 final = (dst1-src2).unit();
	float angle = aCos(initial.dot(final));
	if (Vector3(0,1,0).dot(initial.cross(final)) < 0) {
		angle = -angle;
	}
	Matrix3 rot = Matrix3::fromAxisAngle(Vector3(0,1,0), angle);
	
	CoordinateFrame rotationFrame = CoordinateFrame(src2)*CoordinateFrame(rot)*CoordinateFrame(-src2);
	CoordinateFrame xform  = targetXform.inverse()*rotationFrame;

	CoordinateFrame newFrame = _gfxMgr->getRoomToVirtualSpaceFrame()*xform.inverse();
	if (!testForManipulationOutsideOfBounds(newFrame) && !testForCrazyManipulation(xform)) {
		_gfxMgr->setRoomToVirtualSpaceFrame(newFrame);
		_feedbackWidget->rotateGlobe(rotationFrame);
		_totalRotationAngle+= abs(angle);
		_eventMgr->queueEvent(new Event("scale_action", targetXform));
	}
}

glm::dvec3 OrigAnchoredHCI::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}

void OrigAnchoredHCI::determineTouchToHandCoorespondence(TouchDataRef touch) {
	// Use the distances between touch points and the tracker to identify which hand they belong to.
	float distToLeft = glm::length(glm::dvec3(glm::column(_currentLeftTrackerFrame, 3)) - touch->getCurrRoomPos());
	float distToRight = glm::length(glm::dvec3(glm::column(_currentRightTrackerFrame, 3)) - touch->getCurrRoomPos());
	if(distToLeft < distToRight) {
		touch->setBelongTo(TouchData::LEFT_HAND);
	}
	else {
		touch->setBelongTo(TouchData::RIGHT_HAND);
	}
}
*/