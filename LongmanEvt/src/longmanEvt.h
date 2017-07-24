#ifndef longmanEvt_h__
#define longmanEvt_h__
#define  EvtTYPE1 "_VIEW_UPDATE_EVENT_TYPE_"
#define  EvtTYPE2 "_EXE_COMMAND_EVENT_TYPE_"
//关于Event类的构造;
//事件类型：(目前)包括EvtTYPE1和EvtTYPE2,作为构造函数参数;底层类lmmodel将EvtTYPE1类型Event;
//发送给其派生类lmView类,将EvtTYPE2类型的Event发送给派生类lmController;
//Event中的参数列表类为Paramlist,该列表第一对参数作为标识符,一般恒定为;
//("CommandName","command"),其中,"command"为自定义标识符,lmView类(或lmController类);
//根据"command"回调它们的派生类(从lmmodel到这些类两次派生)函数;
//关于调度:使用函数dispatch()即可
//关于传递:该类提供了clone模式，使用C++自带智能指针shared_ptr模板，进行;
//事件的传递;
#include "EvtParamList.h"
#include <list>
class lmEvtBus;
class longmanEvt
{
public:
	//longmanEvt();
	longmanEvt(const std::string &);
	virtual ~longmanEvt();
	virtual longmanEvt* clone()const { return new longmanEvt(*this); }
private:
	EvtParamList m_paramList;
	std::string m_eventTYPE;

public:
	bool setParam(const std::string&, const lmVariant&);
	longmanEvt &setParam(const std::string&, const lmVariant&,bool ret);
	//ret=true,get success;ret=false,not exist;
	lmVariant getParam(const std::string&);
	bool getParam(const std::string&, lmVariant&);
	void dispatch(lmEvtBus *pcEventBus=nullptr) const;
	bool getParamIter(paramlist::const_iterator&,paramlist::const_iterator&) const;
	std::string getEvtTYPE() const { return m_eventTYPE; }
};

#endif // longmanEvt_h__
