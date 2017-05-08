#include "longmanEvt.h"
#include "lmEvtBus.h"

// longmanEvt::longmanEvt()
// {
// 	m_eventName="NULL";
// }


longmanEvt::longmanEvt(const std::string & pevtName)
{
	m_eventTYPE=pevtName;
}

longmanEvt::~longmanEvt()
{
}
bool longmanEvt::setParam(const std::string& pEvtName, const lmVariant& pParamValue)
{
	return m_paramList.setParam(pEvtName, pParamValue);
}
longmanEvt &longmanEvt::setParam(const std::string& pEvtName, const lmVariant& pParamValue, bool ret)
{
	m_paramList.setParam(pEvtName, pParamValue);
	return *this;
}
//若参数不存在，则返回0;
lmVariant longmanEvt::getParam(const std::string& pParamName)
{
	return m_paramList.getParam(pParamName);
}
bool longmanEvt::getParam(const std::string& pParamName, lmVariant& param)
{
	return m_paramList.getParam(pParamName, param);
}
void longmanEvt::dispatch(lmEvtBus *pcEventBus)const
{
	if (pcEventBus == NULL)
	{
		lmEvtBus::createSingleCon()->post(*this);
	}
	else
	{
		pcEventBus->post(*this);
	}
}

bool longmanEvt::getParamIter(paramlist::const_iterator& begit, paramlist::const_iterator& endit) const
{
	return m_paramList.getParamIter(begit, endit);
}
