
#ifndef lmDecodeThread_h__
#define lmDecodeThread_h__

#include <QThread>
#include <QWaitCondition>
#include "src/lmmodel.h"
#include "src/lmTYPE.h"
#include "..\longmanApp\src\appData\lmParseStreamPro.h"
class lmDecodeThread : public QThread
{
	Q_OBJECT

public:
	lmDecodeThread(QObject *parent = 0);
	~lmDecodeThread();
	bool parseSHVCBitBtream(longmanEvt& rEvt);
	bool addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle);
	EvtQue &getEvtQue() { return evtue; };
	QWaitCondition& getCondition() { return condition; }
	QMutex& getMutx() { return mutex; }
protected:
	void run() Q_DECL_OVERRIDE;

private:
	void handleCmd(longmanEvt& requstCmd);
	EvtQue evtue;
	CallBackFuncList _commandTable;
	QMutex mutex;
	QWaitCondition condition;
};
#endif // lmDecodeThread_h__
