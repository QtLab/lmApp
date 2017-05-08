#ifndef lmView_h__
#define lmView_h__
#include <QObject>
#include "src\lmmodel.h"
#include "src\longmanEvt.h"
//底层类lmmodel的派生类,响应类型为"EvtTYPE1"的Event,该类的回调函数列表作为对;
//该类的派生类的回调函数表,设计该类的目的在于派生View类，提供listenParam()函数;
//设定响应特定的"command";
class lmView:public lmmodel
{
public:
	lmView(lmEvtBus *evtBus=nullptr);
	~lmView();
	void listenParam(const std::string & , const CallBackFunc&);
	bool handlevt(longmanEvt&);
private:
	CallBackFuncList mCallBacklist;
};

#endif // lmView_h__
