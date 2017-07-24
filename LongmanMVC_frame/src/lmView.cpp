#include "lmView.h"
#include<iostream>
lmView::lmView(lmEvtBus *evtBus):lmmodel(evtBus)
{
	//����View��model������˵�������յ�����Ϊ_VIEW_UPDATE_EVENT_TYPE_��;
	//Enentʱ���ص�View��handlevt����;
	CallBackFunc callbackfunc = std::bind(&lmView::handlevt, this, std::placeholders::_1);
	subscribeEvt(EvtTYPE1, callbackfunc);
}


lmView::~lmView()
{
}

void lmView::listenParam(const std::string & rparamName, const CallBackFunc& rcallbakfunc)
{
	//��View��������Ļص����������б�������model��subscribeEvt����
	mCallBacklist.insert(CallBackFuncList::value_type(rparamName, rcallbakfunc));

}

bool lmView::handlevt(longmanEvt& pEvt)
{

	paramlist::const_iterator paramBeg;
	paramlist::const_iterator paramEnd;
	if (!(pEvt.getParamIter(paramBeg, paramEnd)) || paramBeg->first != "CommandName")
	{
		return false;
	}
	std::string &m_cmdName = (paramBeg->second).toString().toStdString();
	auto m_cmdHandlef = mCallBacklist.find(m_cmdName);
	if (m_cmdHandlef != mCallBacklist.end())
	{
		m_cmdHandlef->second(pEvt);
	}
	//CallBackFuncList::iterator callbackIter;
	//��������paramList
	//for (auto paramiter = paramBeg; paramiter != paramEnd;++paramiter)
	//{
	//	//�ڻص�����list�������ڼ�����param;
	//	callbackIter = mCallBacklist.find(paramiter->first);
	//	if (callbackIter!= mCallBacklist.end())
	//	{
	//		//�ص����ڼ�����param��Ӧ�Ļص�����;
	//		callbackIter->second(pEvt);
	//	}
	//}
	return true;
}
