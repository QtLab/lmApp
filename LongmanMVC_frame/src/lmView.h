#ifndef lmView_h__
#define lmView_h__
#include <QObject>
#include "src\lmmodel.h"
#include "src\longmanEvt.h"
//�ײ���lmmodel��������,��Ӧ����Ϊ"EvtTYPE1"��Event,����Ļص������б���Ϊ��;
//�����������Ļص�������,��Ƹ����Ŀ����������View�࣬�ṩlistenParam()����;
//�趨��Ӧ�ض���"command";
class lmView:public lmmodel
{
public:
	explicit lmView(lmEvtBus *evtBus=nullptr);
	~lmView();
	void listenParam(const std::string & , const CallBackFunc&);
	bool handlevt(longmanEvt&);
private:
	CallBackFuncList mCallBacklist;
};

#endif // lmView_h__
