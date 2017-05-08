#ifndef cmdProcessThread_h__
#define cmdProcessThread_h__
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QThread>
#include<list>
// #include <QTGui/QPixmap> 
// #include <QTGui/QImage> 
#include "src/lmTYPE.h"
#include "src/lmmodel.h"
#include "..\longmanApp\src\appData\lmData.h"
#include "..\longmanApp\src\appData\lmImageDraw.h"
typedef std::list<longmanEvt*> EvtQue;
//（目前）作用:在工作线程中处理EvtTYPE2类型的Event,建立简单的Event缓冲机制;
//
struct cyuvParam
{
	int mWidth = 0;
	int mHeight = 0;
	int mFormat = 1;
	int mcurPOC = 0;
	std::string yuvPath;
};
typedef std::function<void(cyuvParam)> normalCallbacfun;
/*class lmData;*/
class cmdProcessThread : public QThread
{
	Q_OBJECT

public:
	cmdProcessThread(QObject *parent = 0);
	~cmdProcessThread();
	bool addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle);
	bool openyuvfile(longmanEvt&);
	bool changeimagepoc(longmanEvt&);
	EvtQue &getEvtQue() { return evtue; };
	QMutex mutex;
	QWaitCondition condition;
	normalCallbacfun recoverhandle;
	const cyuvParam getlastyuvParam() const {return lastyuvParam;};
protected:
	void run() Q_DECL_OVERRIDE;
private:
	void handleCmd(longmanEvt&);
	EvtQue evtue;
	CallBackFuncList _commandTable;
	lmData dataModel;
	QImage mImage;
	cyuvParam lastyuvParam;
	lmImageDraw mImageDraw;
};
#endif // cmdProcessThread_h__
