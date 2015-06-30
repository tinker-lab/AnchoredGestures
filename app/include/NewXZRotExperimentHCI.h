//#ifndef newanchoredexperimenthci_h_
//#define newanchoredexperimenthci_h_
//
//#include "gl/glew.h"
//#include <glfw/glfw3.h>
//#include "abstracthci.h"
//#include "mvrcore/abstractcamera.h"
//#include <mvrcore/cameraoffaxis.h>
//#include "cframemgr.h"
//#include "app/include/gpumesh.h"
//#include "app/include/glslprogram.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_access.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <mvrcore/datafileutils.h>
//#include "app/include/texturemgr.h"
//#include "app/include/touchdata.h"
//#include <glm/gtx/string_cast.hpp>
//#include <boost/algorithm/string/predicate.hpp>
//#include <mvrcore/time.h>
//#include <iterator>
//
//
//class newanchoredexperimenthci : public abstracthci {
//
//public:
//	newanchoredexperimenthci(minvr::abstractcameraref camera, cframemgrref cframemgr, texturemgrref textman, feedbackref feedback);
//	virtual ~newanchoredexperimenthci();
//	void update(const std::vector<minvr::eventref> &events);
//	glm::dvec3 convertscreentoroomcoordinates(glm::dvec2 screencoords);
//	void initializecontextspecificvars(int threadid,minvr::windowref window);
//
//	void closesttouchpair(std::map<int, touchdataref> thisregisteredtouchdata, glm::dvec3 &pos1, glm::dvec3 &pos2, double &mindistance);
//	void updatehandpos(const std::vector<minvr::eventref>& events);
//
//	// matrix transforms
//	void translate(glm::dmat4 transmat);
//	void yrotationandscale(touchdataref centofrotdata, touchdataref othertouchdata);
//	
//
//private:
//	std::shared_ptr<minvr::cameraoffaxis> offaxiscamera;
//	std::map<int, touchdataref> registeredtouchdata; 
//	std::shared_ptr<gpumesh> cubemesh;
//	std::shared_ptr<glslprogram> shader;
//	std::map<int, gluint> _vboid;
//	texturemgrref texman; 
//	minvr::timestamp starttime;
//	double prevscaleby;
//	minvr::eventref hand1;
//	minvr::eventref hand2;
//	glm::dvec3 currhandpos1;
//	glm::dvec3 prevhandpos1;
//	glm::dvec3 currhandpos2;
//	glm::dvec3 prevhandpos2;
//	int numtouchforhand1;
//	int numtouchforhand2;
//	bool xzrotflag;
//	bool initroompos;
//	bool liftedfingers;
//	glm::dvec3 initroomtouchcentre;
//	glm::dvec3 roomtouchcentre;
//	bool centerrotmode;
//	glm::dvec3 currhandtotouch;
//	glm::dvec3 prevhandtotouch;
//
//};
//
//#endif /* newanchoredexperimenthci_h_ */