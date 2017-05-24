#ifndef lmmodelD_h__
#define lmmodelD_h__

#include <QObject>
#include"src\lmTYPE.h"
class lmEvtBus;
class lmmodel;
//typedef bool (*CallBackFunc)(longmanEvt&);
//定义bool (longmanEvt&)类型函数调用入口，用于基类回调派生类;
typedef std::function<bool(longmanEvt&)> CallBackFunc;
typedef std::map<std::string, CallBackFunc> CallBackFuncList;
class lmmodelD : public QObject
{
	Q_OBJECT
friend lmmodel;
private:
	explicit lmmodelD(lmEvtBus *evtBus);
	void subscribeEvt(const std::string&, const CallBackFunc&);
	void unsubcribeEvt(const std::string&);
	void unsubcribeEvt(const longmanEvt&);
public slots:
	void callBackListenEvtFunOfSub(Evt_smartptr reEvt);
private:
	lmEvtBus *m_evtBus;
	CallBackFuncList mCallBacklist;
};
#endif // lmmodelD_h__
