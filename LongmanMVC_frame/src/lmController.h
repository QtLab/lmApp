#ifndef lmController_h__
#define lmController_h__
//#include "src\lmmodel.h"
//#include "src\longmanEvt.h"
#include "src\cmdProcessThread.h"
#include "..\longmanApp\src\appData\lmDecodeThread.h"
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
	bool parseSHVCBitBtream(longmanEvt& rEvt);
private:
	void xcmdInti();
	bool sendMsg(const std::string&);
	void note(bool s);
	//构造函数私有，以配合单例模式;
	explicit lmController(lmEvtBus *evtBus);
	static lmController* _mInstance;
	CallBackFuncList mCallBacklist;
	cmdProcessThread workThread;
	longmanEvt workThreadMsg;
	int maxQue;
	bool EvtQueFull;
	lmDecodeThread pDecoder;
};
#endif // lmController_h__

