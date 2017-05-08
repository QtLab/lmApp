#ifndef lmController_h__
#define lmController_h__
//#include "src\lmmodel.h"
//#include "src\longmanEvt.h"
#include "src\cmdProcessThread.h"
//底层类lmmodel的派生类,响应类型为"EvtTYPE2"的Event,该类的回调函数列表作为对;
//该类的派生类的回调函数表,(目前),类型为"EvtTYPE2"的Event均交由cmdProcessThread类;
//处理;
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

