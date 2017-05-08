#include "lmmodel.h"
#include "lmEvtBus.h"

lmmodel::lmmodel(lmEvtBus *evtBus):
	_modeld(evtBus)
{

}

void lmmodel::subscribeEvt(const std::string& pe, const CallBackFunc& pc)
{
	_modeld.subscribeEvt(pe, pc);
}

void lmmodel::unsubcribeEvt(const std::string& pe)
{
	_modeld.unsubcribeEvt(pe);
}

void lmmodel::unsubcribeEvt(const longmanEvt& pe)
{
	_modeld.unsubcribeEvt(pe);
}

void lmmodel::setModelName(const std::string& rcName)
{
	_modelName = rcName;
}
