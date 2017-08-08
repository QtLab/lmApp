#include "lmView.h"

#include<iostream>
lmView::lmView(lmEvtBus *evtBus):lmmodel(evtBus)
{
	//对于View的model基类来说，当接收到类型为_VIEW_UPDATE_EVENT_TYPE_的;
	//Enent时，回调View的handlevt函数;
	CallBackFunc callbackfunc = std::bind(&lmView::handlevt, this, std::placeholders::_1);
	subscribeEvt(EvtTYPE1, callbackfunc);
}


lmView::~lmView()
{
}

void lmView::listenParam(const std::string & rparamName, const CallBackFunc& rcallbakfunc)
{
	//将View的派生类的回调函数加入列表，类似于model的subscribeEvt函数
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
	//遍历所有paramList
	//for (auto paramiter = paramBeg; paramiter != paramEnd;++paramiter)
	//{
	//	//在回调函数list查找正在监听的param;
	//	callbackIter = mCallBacklist.find(paramiter->first);
	//	if (callbackIter!= mCallBacklist.end())
	//	{
	//		//回调正在监听的param对应的回调函数;
	//		callbackIter->second(pEvt);
	//	}
	//}
	return true;
}
