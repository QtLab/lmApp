#ifndef lmController_h__
#define lmController_h__
//#include "src\lmmodel.h"
//#include "src\longmanEvt.h"
#include "src\cmdProcessThread.h"
//�ײ���lmmodel��������,��Ӧ����Ϊ"EvtTYPE2"��Event,����Ļص������б���Ϊ��;
//�����������Ļص�������,(Ŀǰ),����Ϊ"EvtTYPE2"��Event������cmdProcessThread��;
//����;
class lmController:public lmmodel
{
public:

	~lmController();
	bool handlevt(longmanEvt&);
	void registerCommand(const std::string &, const CallBackFunc&);
	static lmController* getInstance();
	void recoverhandle(cyuvParam&);
private:
	void xcmdInti();
	bool sendMsg(const std::string&);
	lmController(lmEvtBus *evtBus);
	static lmController* _mInstance;
	CallBackFuncList mCallBacklist;
	cmdProcessThread workThread;
	longmanEvt workThreadMsg;
	int maxQue;
	bool EvtQueFull;
};
#endif // lmController_h__

