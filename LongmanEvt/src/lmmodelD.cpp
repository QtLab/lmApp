#include "lmmodelD.h"
#include "lmEvtBus.h"
#include "lmmodel.h"
lmmodelD::lmmodelD(lmEvtBus *evtBus)
{
	if (evtBus==nullptr)
	{
		m_evtBus = lmEvtBus::createSingleCon();
	}
	else
	{
		m_evtBus = evtBus;
	}
	m_evtBus->registerModel(this);
}

// lmmodelD::~lmmodelD()
// {
// }

void lmmodelD::subscribeEvt(const std::string& pevtName, const CallBackFunc& pCallback)
{
	mCallBacklist.insert(CallBackFuncList::value_type(pevtName, pCallback));
}

void lmmodelD::unsubcribeEvt(const std::string& pevtName)
{
	CallBackFuncList::iterator iter;
	iter = mCallBacklist.find(pevtName);
	if (iter != mCallBacklist.end())
	{
		mCallBacklist.erase(iter);
	}
}

void lmmodelD::unsubcribeEvt(const longmanEvt& pevt)
{
	std::string mevtname = pevt.getEvtTYPE();
	CallBackFuncList::iterator iter;
	iter = mCallBacklist.find(mevtname);
	if (iter != mCallBacklist.end())
	{
		mCallBacklist.erase(iter);
	}
}

void lmmodelD::callBackListenEvtFunOfSub(Evt_smartptr reEvt)
{

	auto m_evtName = reEvt->getEvtTYPE();
	CallBackFuncList::iterator iter;
	iter = mCallBacklist.find(m_evtName);
	if (iter != mCallBacklist.end())
	{
		iter->second(*reEvt);
	}
}