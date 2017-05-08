#ifndef lmmodel_h__
#define lmmodel_h__
#include "longmanEvt.h"
#include "lmmodelD.h"
//底层基类,通过组合委托
class lmmodel
{
public:
	lmmodel(lmEvtBus *evtBus);
	/*~lmmodel();*/
	void subscribeEvt(const std::string&, const CallBackFunc&);
	void unsubcribeEvt(const std::string&);
	void unsubcribeEvt(const longmanEvt&);
	void setModelName(const std::string&);
private:
	lmmodelD _modeld;
	std::string _modelName;
};
#endif // lmmodelD_h__
