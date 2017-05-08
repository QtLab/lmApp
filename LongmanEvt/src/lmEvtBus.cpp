#include "lmEvtBus.h"
#include "lmmodelD.h"
lmEvtBus* lmEvtBus::m_SingleCon = nullptr;
lmEvtBus::lmEvtBus()
{
}

lmEvtBus::~lmEvtBus()
{
}

lmEvtBus* lmEvtBus::createCon()
{
	return new lmEvtBus();
}

lmEvtBus* lmEvtBus::createSingleCon()
{
	if (m_SingleCon == NULL)
		m_SingleCon = new lmEvtBus();
	return m_SingleCon;
}

void lmEvtBus::registerModel(lmmodelD* pmodel)
{
	qRegisterMetaType<Evt_smartptr>("Evt_smartptr");
	connect(this, SIGNAL(evtTriggered(Evt_smartptr)), pmodel, SLOT(callBackListenEvtFunOfSub(Evt_smartptr)), Qt::AutoConnection);
}

void lmEvtBus::post(const longmanEvt& pEvt)
{
	Evt_smartptr pEvtCopy(pEvt.clone());
	emit evtTriggered(pEvtCopy);
}
