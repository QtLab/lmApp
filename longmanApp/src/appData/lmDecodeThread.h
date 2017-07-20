
#ifndef lmDecodeThread_h__
#define lmDecodeThread_h__

#include <QThread>
#include <QWaitCondition>
#include "src/lmmodel.h"
#include "src/lmTYPE.h"
class lmDecodeThread : public QThread
{
	Q_OBJECT

public:
	lmDecodeThread(QObject *parent);
	~lmDecodeThread();
	bool parseSHVCBitBtream(longmanEvt& rEvt);
	bool addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle);
protected:
	void run() Q_DECL_OVERRIDE;
	EvtQue evtue;
private:
	void handleCmd(longmanEvt&);
	EvtQue evtue;
	CallBackFuncList _commandTable;
	QMutex mutex;
	QWaitCondition condition;

};
#endif // lmDecodeThread_h__
