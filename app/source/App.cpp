#include <app/include/App.h>



using namespace MinVR;

App::App() : MinVR::AbstractMVRApp() {
	
	_replayingStream = false;
	std::string ignoreString = ConfigVal("LoggingEventsToIgnore", "", false);
	while (ignoreString.size()) {
		std::string ignoreEvent;
		if (MinVR::popNextToken(ignoreString, ignoreEvent, true)) {
			_logIgnoreList.push_back(ignoreEvent);
		}
		else {
			break;
		}
	}

	boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
	facet->format("%Y-%m-%d.%H.%M.%S");
	std::stringstream stream;
	stream.imbue(std::locale(stream.getloc(), facet));
	stream << boost::posix_time::second_clock::local_time();
	std::string eventStreamFile = "EventsText-" + stream.str() + ".txt";
	_eventsForText.open(eventStreamFile);

}

App::~App() {
	for(std::map<int, GLuint>::iterator iterator = _vboId.begin(); iterator != _vboId.end(); iterator++) {
		glDeleteBuffersARB(1, &iterator->second);
	}

	_eventsForText.close();
}

void App::generateTrials()
{
	using namespace MinVR;
	srand (static_cast <unsigned> (time(0)));
	std::ofstream trials("TrialGeneration.txt");
	int numTrials = MinVR::ConfigVal("NumTrials", 8, false);
	for(int i=0; i < numTrials; i++) {
		double randVal = -0.25 + (double)rand()/(double)(RAND_MAX/ 0.5);//(-0.25, 0.25); // randomly move within 3 in.
		glm::dmat4 transform = glm::translate(glm::dmat4(1.0), glm::dvec3(0.0, randVal, 0.0));
		trials << "TransMat"<<i+1<<" "<<transform<<std::endl;
	}
	trials <<std::endl;

	for(int i=0; i < numTrials; i++) {
		glm::dvec2 circleVec = glm::circularRand(1.0);
		glm::dvec3 axis = glm::normalize(glm::dvec3(circleVec.x, 0.0, circleVec.y));
		double angle = -45 + (double)rand()/(double)(RAND_MAX/ 90); // -45 to 45
		glm::dmat4 transform = glm::rotate(glm::dmat4(1.0), angle, axis);
		trials<<"# Rotate: "<<axis<< " by "<<angle<<" degrees"<<std::endl;
		trials << "RotMat"<<i+1<<" "<<transform<<std::endl;
	}
	trials <<std::endl;

	for(int i=0; i < numTrials; i++) {
		glm::dvec3 axis = glm::sphericalRand(1.0);
		double angle = -45 + (double)rand()/(double)(RAND_MAX/ 90); // -45 to 45
		glm::dmat4 transform = glm::rotate(glm::dmat4(1.0), angle, axis);
		int doScale = rand()%2;
		double scaleVal = 1.0;
		if (doScale) {
			scaleVal = 0.5 + (double)rand()/(double)(RAND_MAX); // 0.5 to 1.5
			transform = glm::scale(transform, glm::dvec3(scaleVal));
		}
		double x = -0.25 + (double)rand()/(double)(RAND_MAX/ 0.5);//(-0.25, 0.25);
		double y = -0.25 + (double)rand()/(double)(RAND_MAX/ 0.5);//(-0.25, 0.25);
		double z = -0.25 + (double)rand()/(double)(RAND_MAX/ 0.5);//(-0.25, 0.25);
		glm::dvec3 translation = glm::dvec3(x, y, z); // randomly move within 3 in.
		transform = glm::translate(transform, translation);
		trials<<"# Rotate: "<<axis<< " by "<<angle<<" degrees; Scale: "<<scaleVal<<" Translate: "<<translation<<std::endl;
		trials << "CombinedMat"<<i+1<<" "<<transform<<std::endl;
	}
	trials <<std::endl;
	std::flush(trials);
	trials.close();
}

void App::saveEventStream(const std::string &eventStreamFilename)
{
	if (_replayingStream) {
		return;
	}

	long totalSize = sizeof(int);
	for(int i=0; i < _eventsToSave.size(); i++){
		totalSize += _eventsToSave[i].getSize();
	}

	ByteStream stream(totalSize);
	long numEvents = _eventsToSave.size();
	std::cout<<"Saving "<<numEvents<<" events"<<std::endl;
	stream.writeInt(numEvents);

	for(int i=0; i < _eventsToSave.size(); i++){
		//if (i > _eventsToSave.size() - 100) {
		//	std::cout<< byteDataToEvent(_eventsToSave[i])->toString()<<std::endl;
		//}
		stream.writeByteData(_eventsToSave[i]);
	}

	FILE * pFile;
	unsigned char* buffer = stream.getByteArray();
	pFile = fopen (eventStreamFilename.c_str(), "wb");
	fwrite (buffer , sizeof(unsigned char), stream.getSize(), pFile);
	fclose (pFile);
}

void App::loadEventStream(const std::string &eventStreamFilename)
{
	FILE * pFile;
	long lSize;
	unsigned char * buffer;
	size_t result;

	pFile = fopen ( eventStreamFilename.c_str() , "rb" );
	if (pFile==NULL) {
		std::cout<<"Error: unable to open "<<eventStreamFilename<<std::endl;
		return;
	}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	rewind (pFile);

	// allocate memory to contain the whole file:
	buffer = new unsigned char[lSize];

	// copy the file into the buffer:
	result = fread (buffer, 1, lSize, pFile);
	if (result != lSize) {
		std::cout<<"Error: unable to read "<<eventStreamFilename<<std::endl;
		return;
	}
	fclose (pFile);

	ByteStream stream(buffer, lSize);
	replayEventStream(stream);

	delete [] buffer;
}

void App::replayEventStream(ByteStream stream)
{
	int numEvents = stream.readInt();

	_replayingStream = true;

	//Throw all of the current state away
	_eventsToSave.clear();

	//_eventsForText << "---------------------- REPLAYING ------------------------"<<std::endl;

	std::vector<EventRef> queue;

	std::cout<<"Replaying "<<numEvents<<" events."<<std::endl;
	for(int i=0; i < numEvents; i++) {
		ByteData data = stream.readByteData();
		//if (i > numEvents - 1000) {
		//	std::cout<< byteDataToEvent(data)->toString()<<std::endl;
		//	std::flush(std::cout);
		//}
		MinVR::EventRef event = byteDataToEvent(data);
		if (event->getName() == "FinishedQueue") {
			doUserInputAndPreDrawComputation(queue, 0.0);
			queue.clear();
		}
		else {
			queue.push_back(event);
		}
	}
	doUserInputAndPreDrawComputation(queue, 0.0);

	_replayingStream = false;
}

/** Each time you change the format of this, increase the
    SERIALIZE_VERSION_NUMBER and respond accordingly in the
    deserialize method.  In this method, you can safely get rid of the
    old code because we don't want to create files in an old format.
*/
#define SERIALIZE_VERSION_NUMBER 0
ByteData App::eventToByteData(MinVR::EventRef event)
{
	ByteStream stream;
	stream.writeInt(SERIALIZE_VERSION_NUMBER);

	std::string name = event->getName();
	int id = event->getId();

	int sizeInBytes = sizeof(int) + sizeof(int) + (sizeof(char)*name.size()+sizeof(int)) + sizeof(int); //version number, eventType, name size int, name, id

	/*
		Note: This does not keep track of the timestamps or window points
	*/

	switch(event->getType()) {
		case MinVR::Event::EVENTTYPE_STANDARD:
		{
			stream.resize(sizeInBytes);
			stream.writeInt(event->getType());
			stream.writeString(name);
			stream.writeInt(id);
			break;
		}
		case MinVR::Event::EVENTTYPE_1D:
		{
			sizeInBytes+= sizeof(double);
			stream.resize(sizeInBytes);
			stream.writeInt(event->getType());
			stream.writeString(name);
			stream.writeInt(id);
			stream.writeDouble(event->get1DData());
			break;
		}
		case MinVR::Event::EVENTTYPE_2D:
		{
			sizeInBytes+= (sizeof(double) * 2);
			stream.resize(sizeInBytes);
			stream.writeInt(event->getType());
			stream.writeString(name);
			stream.writeInt(id);
			glm::dvec2 data = event->get2DData();
			double* dataPtr = glm::value_ptr<double>(data);
			stream.writeDouble(*dataPtr);
			dataPtr++;
			stream.writeDouble(*dataPtr);
			break;
		}
		case MinVR::Event::EVENTTYPE_3D:
		{
			sizeInBytes+= (sizeof(double) * 3);
			stream.resize(sizeInBytes);
			stream.writeInt(event->getType());
			stream.writeString(name);
			stream.writeInt(id);
			glm::dvec3 data = event->get3DData();
			double* dataPtr = glm::value_ptr<double>(data);
			stream.writeDouble(*dataPtr);
			dataPtr++;
			stream.writeDouble(*dataPtr);
			dataPtr++;
			stream.writeDouble(*dataPtr);
			dataPtr++;
			break;
		}
		case MinVR::Event::EVENTTYPE_4D:
		{
			sizeInBytes+= (sizeof(double) * 4);
			stream.resize(sizeInBytes);
			stream.writeInt(event->getType());
			stream.writeString(name);
			stream.writeInt(id);
			glm::dvec4 data = event->get4DData();
			double* dataPtr = glm::value_ptr<double>(data);
			stream.writeDouble(*dataPtr);
			dataPtr++;
			stream.writeDouble(*dataPtr);
			dataPtr++;
			stream.writeDouble(*dataPtr);
			dataPtr++;
			stream.writeDouble(*dataPtr);
			break;
		}
		case MinVR::Event::EVENTTYPE_COORDINATEFRAME:
		{
			sizeInBytes+= (sizeof(double) * 16);
			stream.resize(sizeInBytes);
			stream.writeInt(event->getType());
			stream.writeString(name);
			stream.writeInt(id);
			glm::dmat4 frame = event->getCoordinateFrameData();
			double* data = glm::value_ptr<double>(frame);
			//std::cout<<"Writing: ";
			for(int i=0; i < 16; i++) {
			//	std::cout<<*data<<" ";
				stream.writeDouble(*data);
				data++;
			}
			//std::cout<<std::endl;
			break;
		}
		case MinVR::Event::EVENTTYPE_MSG:
		{
			std::string msg = event->getMsgData();
			sizeInBytes+= (sizeof(char)*msg.size()+sizeof(int));
			stream.resize(sizeInBytes);
			stream.writeInt(event->getType());
			stream.writeString(name);
			stream.writeInt(id);
			stream.writeString(msg);
			break;
		}
	}
	
	return stream.toByteData();
}

/** Never delete the code from an old version from this method so we
    can always stay backwards compatable.  Important, since this is in
    binary, and we can't really reverse engineer the file format.
*/
MinVR::EventRef App::byteDataToEvent(ByteData data)
{
	ByteStream stream(data);
	int version = stream.readInt();

	if (version == 0) {
		MinVR::Event::EventType type = static_cast<MinVR::Event::EventType>(stream.readInt());
		std::string name = stream.readString();
		int id = stream.readInt();

		switch(type) {
			case MinVR::Event::EVENTTYPE_STANDARD:
			{
				return MinVR::EventRef(new Event(name, nullptr, id));
			}
			case MinVR::Event::EVENTTYPE_1D:
			{
				double data = stream.readDouble();
				return MinVR::EventRef(new Event(name, data, nullptr, id));
			}
			case MinVR::Event::EVENTTYPE_2D:
			{
				double data[2];
				data[0] = stream.readDouble();
				data[1] = stream.readDouble();
				return MinVR::EventRef(new Event(name, glm::make_vec2<double>(data), nullptr, id));
			}
			case MinVR::Event::EVENTTYPE_3D:
			{
				double data[3];
				data[0] = stream.readDouble();
				data[1] = stream.readDouble();
				data[2] = stream.readDouble();
				return MinVR::EventRef(new Event(name, glm::make_vec3<double>(data), nullptr, id));
			}
			case MinVR::Event::EVENTTYPE_4D:
			{
				double data[4];
				data[0] = stream.readDouble();
				data[1] = stream.readDouble();
				data[2] = stream.readDouble();
				data[3] = stream.readDouble();
				return MinVR::EventRef(new Event(name, glm::make_vec4<double>(data), nullptr, id));
			}
			case MinVR::Event::EVENTTYPE_COORDINATEFRAME:
			{
				double data[16];
				//std::cout<<"Reading: ";
				for(int i=0; i < 16; i++) {
						data[i] = stream.readDouble();
				//		std::cout<<data[i]<<" ";
				}
				//std::cout<<std::endl;
				return MinVR::EventRef(new Event(name, glm::make_mat4x4<double>(data), nullptr, id));
			}
			case MinVR::Event::EVENTTYPE_MSG:
			{
				std::string msg = stream.readString();
				return MinVR::EventRef(new Event(name, msg, nullptr, id));
			}
			default:
			{
				std::cout<<"ERROR: found MinVR event type that is undefined"<<std::endl;
				return MinVR::EventRef(new Event("ERROR"));
			}
		}
	}
	else {
		std::cout<<"ERROR: Undefined bytedata version number"<<std::endl;
		return MinVR::EventRef(new Event("ERROR"));
	}
}


void App::doUserInputAndPreDrawComputation(const std::vector<MinVR::EventRef>& events, double synchronizedTime) {

	//if (_replayingStream && synchronizedTime != 0.0) {
	//	std::cout<<"Replaying "<<events.size()<<" events" <<std::endl;
	//}

	for(int i=0; i < events.size(); i++) {
		if (events[i]->getName() == "kbd_ESC_down") {
			exit(0);
		}
		else if (events[i]->getName() == "kbd_SPACE_down") {
			cFrameMgr->setRoomToVirtualSpaceFrame(glm::dmat4(1.0)); 
		}
		else if (events[i]->getName() == "kbd_G_down") {
			generateTrials();
		}
		else if (events[i]->getName() == "kbd_S_down" && !_replayingStream) {
			std::string eventStreamFile = MinVR::ConfigVal("EventStreamFilePrefix", "EventStream", false);
			boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
			facet->format("%Y-%m-%d.%H.%M.%S");
			std::stringstream stream;
			stream.imbue(std::locale(stream.getloc(), facet));
			stream << boost::posix_time::second_clock::local_time();
			eventStreamFile += "-" + stream.str() + ".bin";
			saveEventStream(eventStreamFile);
		}
		else if (events[i]->getName() == "kbd_R_down") {
			std::string eventStreamFile = MinVR::ConfigVal("EventStreamToReplay", "EventStream", false);
			loadEventStream(eventStreamFile);
		}
		else if (events[i]->getName() == "kbd_N_down") {
		
			experimentMgr->advance(newOld);
			if (experimentMgr->HCIExperiment == 0) {
				currentHCIMgr->currentHCI = likertHCI;
			} else if (experimentMgr->HCIExperiment == 1) {
				currentHCIMgr->currentHCI = newYTransHCI;
			} else if (experimentMgr->HCIExperiment == 2) {
				currentHCIMgr->currentHCI = newXZRotHCI;
			} else if (experimentMgr->HCIExperiment == 3) {
				currentHCIMgr->currentHCI = newAnchoredHCI;
			} else if (experimentMgr->HCIExperiment == 4) {
				currentHCIMgr->currentHCI = origYTransHCI;
			} else if (experimentMgr->HCIExperiment == 5) {
				currentHCIMgr->currentHCI = origXZRotHCI;
			} else if (experimentMgr->HCIExperiment == 6) {
				currentHCIMgr->currentHCI = origAnchoredHCI;
			}
		}
		//Save to the bytestream
		if (std::find(_logIgnoreList.begin(), _logIgnoreList.end(), events[i]->getName()) == _logIgnoreList.end()) {
			_eventsToSave.push_back(eventToByteData(events[i]));
		}

		//_eventsForText << events[i]->toString() << std::endl;

	}

	MinVR::EventRef finishEvent(new Event("FinishedQueue", glfwGetTime()));
	_eventsToSave.push_back(eventToByteData(finishEvent));
	//_eventsForText << finishEvent->toString() << std::endl;
	

	currentHCIMgr->currentHCI->update(events);
	//local time stamp to check how long user taking
	MinVR::TimeStamp check = getCurrentTime();
	std::cout<<"trialtimelimit"<<trailTimeLimit<<std::endl;
	std::cout<<"trialStart: "<<experimentMgr->trialStart<<std::endl;
	std::cout<<"check: "<<check<<std::endl;
	double currentLengthOfTrial = (getDuration(check, experimentMgr->trialStart)).total_milliseconds();
	std::cout<<"currentLengthOfTrial: "<<currentLengthOfTrial<<std::endl;


	if(experimentMgr->HCIExperiment != 0){
		if (experimentMgr->checkFinish() || currentLengthOfTrial > trailTimeLimit ) {

			experimentMgr->advance(newOld);
			if (experimentMgr->HCIExperiment == 0) {
				currentHCIMgr->currentHCI = likertHCI;
			} else if (experimentMgr->HCIExperiment == 1) {
				currentHCIMgr->currentHCI = newYTransHCI;
			} else if (experimentMgr->HCIExperiment == 2) {
				currentHCIMgr->currentHCI = newXZRotHCI;
			} else if (experimentMgr->HCIExperiment == 3) {
				currentHCIMgr->currentHCI = newAnchoredHCI;
			} else if (experimentMgr->HCIExperiment == 4) {
				currentHCIMgr->currentHCI = origYTransHCI;
			} else if (experimentMgr->HCIExperiment == 5) {
				currentHCIMgr->currentHCI = origXZRotHCI;
			} else if (experimentMgr->HCIExperiment == 6) {
				currentHCIMgr->currentHCI = origAnchoredHCI;
			}


		}
	}
	else{
		if (experimentMgr->checkFinish()) {

			experimentMgr->advance(newOld);
			if (experimentMgr->HCIExperiment == 0) {
				currentHCIMgr->currentHCI = likertHCI;
			} else if (experimentMgr->HCIExperiment == 1) {
				currentHCIMgr->currentHCI = newYTransHCI;
			} else if (experimentMgr->HCIExperiment == 2) {
				currentHCIMgr->currentHCI = newXZRotHCI;
			} else if (experimentMgr->HCIExperiment == 3) {
				currentHCIMgr->currentHCI = newAnchoredHCI;
			} else if (experimentMgr->HCIExperiment == 4) {
				currentHCIMgr->currentHCI = origYTransHCI;
			} else if (experimentMgr->HCIExperiment == 5) {
				currentHCIMgr->currentHCI = origXZRotHCI;
			} else if (experimentMgr->HCIExperiment == 6) {
				currentHCIMgr->currentHCI = origAnchoredHCI;
			}


		}
	}
}

void App::initializeContextSpecificVars(int threadId,
		MinVR::WindowRef window) {


	
	
	////////////////////////////////////////////////////
	///where we manually swith two overall experiments//
	////////////////////////////////////////////////////
	newOld = true;	
	////////////////////////////////////////////////////
	///where we manually swith two overall experiments//
	////////////////////////////////////////////////////
	
	trailTimeLimit = MinVR::ConfigVal("TrialTimeLimit", 50000, false); 

	texMan.reset(new TextureMgr());
	texMan->loadTexturesFromConfigVal(threadId, "LoadTextures");

	offAxisCamera = std::dynamic_pointer_cast<MinVR::CameraOffAxis>(window->getCamera(0));

	cFrameMgr.reset(new CFrameMgr());
	feedback.reset(new Feedback(window->getCamera(0), cFrameMgr, texMan));

	currentHCIMgr.reset(new CurrentHCIMgr());
	currentHCIMgr->currentHCI.reset(new TestHCI(window->getCamera(0), cFrameMgr, texMan, feedback));//create promptHCI then reset later

	likertHCI.reset(new LikertHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	newYTransHCI.reset(new NewYTransExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	newXZRotHCI.reset(new NewXZRotExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	newAnchoredHCI.reset(new NewAnchoredExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	origYTransHCI.reset(new OrigYTransExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	origXZRotHCI.reset(new OrigXZRotExperimentHCI(window->getCamera(0), cFrameMgr, texMan, feedback));
	origAnchoredHCI.reset(new OrigAnchoredHCI(window->getCamera(0), cFrameMgr, texMan, feedback));

	likertHCI->initializeContextSpecificVars(threadId, window);
	newYTransHCI->initializeContextSpecificVars(threadId, window);
	newXZRotHCI->initializeContextSpecificVars(threadId, window);
	newAnchoredHCI->initializeContextSpecificVars(threadId, window);
	origYTransHCI->initializeContextSpecificVars(threadId, window);
	origXZRotHCI->initializeContextSpecificVars(threadId, window);
	origAnchoredHCI->initializeContextSpecificVars(threadId, window);

	axis.reset(new Axis(window->getCamera(0), cFrameMgr, texMan));

	experimentMgr.reset(new ExperimentMgr(currentHCIMgr, cFrameMgr, window->getCamera(0), texMan, feedback));
	

	initGL();
	initVBO(threadId, window);
	initLights();

	axis->initializeContextSpecificVars(threadId, window);
	feedback->initializeContextSpecificVars(threadId, window);
	experimentMgr->initializeContextSpecificVars(threadId, window);
	
	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}
	
	
}


void App::initVBO(int threadId, MinVR::WindowRef window)
{
	// cube ///////////////////////////////////////////////////////////////////////
	//    v6----- v5
	//   /|      /|
	//  v1------v0|
	//  | |     | |
	//  | |v7---|-|v4
	//  |/      |/
	//  v2------v3

	// vertex coords array for glDrawArrays() =====================================
	// A cube has 6 sides and each side has 2 triangles, therefore, a cube consists
	// of 36 vertices (6 sides * 2 tris * 3 vertices = 36 vertices). And, each
	// vertex is 3 components (x,y,z) of floats, therefore, the size of vertex
	// array is 108 floats (36 * 3 = 108).
	GLfloat vertices[]  = { 1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f,-1.0f, 1.0f,      // v0-v1-v2 (front)
						   -1.0f,-1.0f, 1.0f,   1.0f,-1.0f, 1.0f,   1.0f, 1.0f, 1.0f,      // v2-v3-v0

							1.0f, 1.0f, 1.0f,   1.0f,-1.0f, 1.0f,   1.0f,-1.0f,-1.0f,      // v0-v3-v4 (right)
							1.0f,-1.0f,-1.0f,   1.0f, 1.0f,-1.0f,   1.0f, 1.0f, 1.0f,      // v4-v5-v0

							1.0f, 1.0f, 1.0f,   1.0f, 1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,      // v0-v5-v6 (top)
						   -1.0f, 1.0f,-1.0f,  -1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,      // v6-v1-v0

						   -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f,-1.0f,  -1.0f,-1.0f,-1.0f,      // v1-v6-v7 (left)
						   -1.0f,-1.0f,-1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,      // v7-v2-v1.0

						   -1.0f,-1.0f,-1.0f,   1.0f,-1.0f,-1.0f,   1.0f,-1.0f, 1.0f,      // v7-v4-v3 (bottom)
							1.0f,-1.0f, 1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f,-1.0f,-1.0f,      // v3-v2-v7

							1.0f,-1.0f,-1.0f,  -1.0f,-1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,      // v4-v7-v6 (back)
						   -1.0f, 1.0f,-1.0f,   1.0f, 1.0f,-1.0f,   1.0f,-1.0f,-1.0f };    // v6-v5-v4

	// normal array
	GLfloat normals[]   = { 0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
							0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0

							1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
							1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0

							0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
							0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0

						   -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
						   -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1

							0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
							0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7

							0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
							0, 0,-1,   0, 0,-1,   0, 0,-1		// v6-v5-v4
	};

	// color array
	GLfloat colors[]    = { 1, 1, 1,   1, 1, 0,   1, 0, 0,      // v0-v1-v2 (front)
							1, 0, 0,   1, 0, 1,   1, 1, 1,      // v2-v3-v0

							1, 1, 1,   1, 0, 1,   0, 0, 1,      // v0-v3-v4 (right)
							0, 0, 1,   0, 1, 1,   1, 1, 1,      // v4-v5-v0

							1, 1, 1,   0, 1, 1,   0, 1, 0,      // v0-v5-v6 (top)
							0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

							1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
							0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

							0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
							1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

							0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
							0, 1, 0,   0, 1, 1,   0, 0, 1,


							0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

							1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
							0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

							0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
							1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

							0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
							0, 1, 0,   0, 1, 1,   0, 0, 1};    // v6-v5-v4

	//making cubeData
	std::vector<int> cubeIndices;
	std::vector<GPUMesh::Vertex> cubeData;
	GPUMesh::Vertex vert;
	for(int i=0; i < 108; i = i +3){
		vert.position = glm::dvec3(vertices[i],vertices[i+1],vertices[i+2]);
		vert.normal = glm::normalize(glm::dvec3(normals[i],normals[i+1],normals[i+2]));
		vert.texCoord0 = glm::dvec2(colors[i],colors[i+1]);
		cubeData.push_back(vert);
		cubeIndices.push_back(cubeData.size()-1);
	}
	
	//initialize Mesh Object

	cubeMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*cubeData.size(), sizeof(int)*cubeIndices.size(),0,cubeData,sizeof(int)*cubeIndices.size(), &cubeIndices[0]));

	// for the background
	std::vector<int> bgQuadIndices;
	std::vector<GPUMesh::Vertex> bgQuadData;
	GPUMesh::Vertex bgQuadVert;

	float windowHeight = window->getHeight();
	float windowWidth = window->getWidth();

	glm::dvec3 bgQuad = glm::abs(convertScreenToRoomCoordinates(glm::dvec2(1.0, 1.0))); // fill up screen size

	bgQuadVert.position = glm::dvec3(-bgQuad.x, 0.0, -bgQuad.z);
	bgQuadVert.normal = glm::dvec3(0.0, 1.0, 0.0);
	bgQuadVert.texCoord0 = glm::dvec2(0.0, 1.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);

	bgQuadVert.position = glm::dvec3(-bgQuad.x, 0.0, bgQuad.z);
	bgQuadVert.texCoord0 = glm::dvec2(0.0, 0.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);

	bgQuadVert.position = glm::dvec3(bgQuad.x, 0.0, -bgQuad.z);
	bgQuadVert.texCoord0 = glm::dvec2(1.0, 1.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);


	bgQuadVert.position = glm::dvec3(bgQuad.x, 0.0, bgQuad.z);
	bgQuadVert.texCoord0 = glm::dvec2(1.0, 0.0);
	bgQuadData.push_back(bgQuadVert);
	bgQuadIndices.push_back(bgQuadData.size()-1);

	bgMesh.reset(new GPUMesh(GL_STATIC_DRAW, sizeof(GPUMesh::Vertex)*bgQuadData.size(), sizeof(int)*bgQuadIndices.size(),0,bgQuadData,sizeof(int)*bgQuadIndices.size(), &bgQuadIndices[0]));

	//axis->initVBO(0);
	//feedback->initVBO(threadId, window);

    // create vertex buffer objects, you need to delete them when program exits
    // Try to put both vertex coords array, vertex normal array and vertex color in the same buffer object.
    // glBufferDataARB with NULL pointer reserves only memory space.
    // Copy actual data with 2 calls of glBufferSubDataARB, one for vertex coords and one for normals.
    // target flag is GL_ARRAY_BUFFER_ARB, and usage flag is GL_STATIC_DRAW_ARB
	_vboId[threadId] = GLuint(0);
	//glGenBuffersARB(1, &_vboId[threadId]);
 //   glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vboId[threadId]);
 //   glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_STATIC_DRAW_ARB);
 //   glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(vertices), vertices);                             // copy vertices starting from 0 offest
 //   glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices), sizeof(normals), normals);                // copy normals after vertices
 //   glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);  // copy colours after normals


	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initVBO: "<<err<<std::endl;
	}
}



void App::initGL()
{
	glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

	glm::dvec3 clearColor = MinVR::ConfigVal("ClearColor", glm::dvec3(0.1), false);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);
	
	//load in shaders
	std::map<std::string, std::string> args, dummyArgs;
	shader.reset(new GLSLProgram());
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	shader->compileShader(MinVR::DataFileUtils::findDataFile("phong.frag").c_str(), GLSLShader::FRAGMENT, args);
	shader->link();

	bgShader.reset(new GLSLProgram());
	bgShader->compileShader(MinVR::DataFileUtils::findDataFile("tex.vert").c_str(), GLSLShader::VERTEX, dummyArgs);
	bgShader->compileShader(MinVR::DataFileUtils::findDataFile("tex.frag").c_str(), GLSLShader::FRAGMENT, args);
	bgShader->link();

	//initGL for the HCIs
	currentHCIMgr->currentHCI->initGL();


	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initGL: "<<err<<std::endl;
	}
}

void App::initLights()
{
	// set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0.5, 0, 3, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initLights: "<<err<<std::endl;
	}
}

glm::dvec3 App::convertScreenToRoomCoordinates(glm::dvec2 screenCoords) {
	glm::dvec3 xVec = offAxisCamera->getTopRight() - offAxisCamera->getTopLeft();
	glm::dvec3 yVec = offAxisCamera->getBottomRight() - offAxisCamera->getTopRight();
	return offAxisCamera->getTopLeft() + (screenCoords.x * xVec) + (screenCoords.y * yVec);
}

void App::postInitialization() {
}

void App::drawGraphics(int threadId, MinVR::AbstractCameraRef camera,
		MinVR::WindowRef window) {
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR: "<<err<<std::endl;
	}

	

	const int numCubeIndices = (int)(cubeMesh->getFilledIndexByteSize()/sizeof(int));
	const int numBgQuadIndices = (int)(bgMesh->getFilledIndexByteSize()/sizeof(int));
	//glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vboId[threadId]);

    // enable vertex arrays
  /*  glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);*/

    // before draw, specify vertex and index arrays with their offsets
 //   glNormalPointer(GL_FLOAT, 0, (void*)(sizeof(GLfloat)*108));
 //   glColorPointer(3, GL_FLOAT, 0, (void*)((sizeof(GLfloat)*108)+(sizeof(GLfloat)*108)));
 //   glVertexPointer(3, GL_FLOAT, 0, 0);

	//glm::dmat4 translate = glm::translate(glm::dmat4(1.0f), glm::dvec3(0.0f, 0.0f, -5.0f));
	//glm::dvec2 rotAngles(-20.0, 45.0);
	//glm::dmat4 rotate1 = glm::rotate(translate, rotAngles.y, glm::dvec3(0.0,1.0,0.0));
	//camera->setObjectToWorldMatrix(glm::rotate(rotate1, rotAngles.x, glm::dvec3(1.0,0,0)));
	/*camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());*/
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	MinVR::CameraOffAxis* offAxisCam = dynamic_cast<MinVR::CameraOffAxis*>(camera.get());

	/////////////////////////////
	// Draw Background Picture //
	/////////////////////////////
	/*bgShader->use();
	bgShader->setUniform("projection_mat", offAxisCam->getLastAppliedProjectionMatrix());
	bgShader->setUniform("view_mat", offAxisCam->getLastAppliedViewMatrix());
	bgShader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
	texMan->getTexture(threadId, "back1")->bind(4);
	bgShader->setUniform("textureSampler", 4);
	camera->setObjectToWorldMatrix(glm::dmat4(1.0));
	bgShader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	glBindVertexArray(bgMesh->getVAOID());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numBgQuadIndices);*/

	
	
	

	/////////////////////////////
	// General Things          //
	/////////////////////////////
	shader->use();
	shader->setUniform("projection_mat", offAxisCam->getLastAppliedProjectionMatrix());
	shader->setUniform("view_mat", offAxisCam->getLastAppliedViewMatrix());
	shader->setUniform("model_mat", offAxisCam->getLastAppliedModelMatrix());
	//shader->setUniform("normal_matrix", glm::dmat3(offAxisCam->getLastAppliedModelMatrix()));
	glm::dvec3 eye_world = glm::dvec3(glm::column(glm::inverse(offAxisCam->getLastAppliedViewMatrix()), 3));
	shader->setUniform("eye_world", eye_world);

	/////////////////////////////
	// Draw Cube               //
	/////////////////////////////
	camera->setObjectToWorldMatrix(cFrameMgr->getVirtualToRoomSpaceFrame());
	shader->setUniform("model_mat", offAxisCamera->getLastAppliedModelMatrix());
	texMan->getTexture(threadId, "Koala1")->bind(0);
	/*glBindVertexArray(cubeMesh->getVAOID());
	glDrawArrays(GL_TRIANGLES, 0, numCubeIndices);*/

	/////////////////////////////
	// Draw Current HCI Stuff  //
	/////////////////////////////
	//currentHCIMgr->currentHCI->draw(threadId,camera,window);	
	
	/////////////////////////////
	// Draw Experiment Things  //
	/////////////////////////////
	experimentMgr->draw(threadId, camera, window);

	/////////////////////////////
	// Draw Axes               // // These use the tex shader, not the phong shaders
	/////////////////////////////
	glm::dvec4 cornerTranslate(-1.7, 0.0, 0.95, 1.0); // modify fourth column
	glm::dmat4 scaleAxisMat = glm::scale(
			glm::dmat4(1.0),
			glm::dvec3(0.1)); 

	//draw x axis
	glm::dmat4 xAxisAtCorner = cFrameMgr->getVirtualToRoomSpaceFrame() * scaleAxisMat;
	xAxisAtCorner[3] = cornerTranslate; // modify fourth column
	camera->setObjectToWorldMatrix(xAxisAtCorner);
	axis->draw(threadId, camera, window, "red");
	
	//draw y axis
	glm::dmat4 yAxisRotMat = glm::rotate(glm::dmat4(1.0), 90.0, glm::dvec3(0.0, 0.0, 1.0));
	glm::dmat4 yAxisAtCorner = (cFrameMgr->getVirtualToRoomSpaceFrame()* scaleAxisMat *yAxisRotMat);
	yAxisAtCorner[3] = cornerTranslate; // modify fourth column
	camera->setObjectToWorldMatrix(yAxisAtCorner);
	axis->draw(threadId, camera, window, "green");

	//// z-axis
	glm::dmat4 zAxisRotMat = glm::rotate(glm::dmat4(1.0), -90.0, glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 zAxisAtCorner = (cFrameMgr->getVirtualToRoomSpaceFrame() * scaleAxisMat * zAxisRotMat);
	zAxisAtCorner[3] = cornerTranslate; // modify fourth column
	camera->setObjectToWorldMatrix(zAxisAtCorner);
	axis->draw(threadId, camera, window, "blue");

	// draw text (actually just a texture)
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/////////////////////////////
	// Draw Visual Feedback    //
	/////////////////////////////
	feedback->draw(threadId, camera, window);

	

 ////   glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
 //   glDisableClientState(GL_COLOR_ARRAY);
 //   glDisableClientState(GL_NORMAL_ARRAY);

 //   glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	/*
	camera->setObjectToWorldMatrix(glm::mat4(1.0));
	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-0.3f, -0.2f, -1.f);
	glColor3f(0.f, 1.0f, 0.f);
	glVertex3f(0.3f, -0.2f, -1.0f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.3f, -1.f);
	glEnd();
	*/
	//glBindVertexArray(cubeMesh->getVAOID());
	//glDrawArrays(GL_TRIANGLES, 0, numCubeIndices);


	

}

