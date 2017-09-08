#ifndef cmdProcessThread_h__
#define cmdProcessThread_h__
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QThread>
#include<list>
#include <memory>
#include "src/lmTYPE.h"
#include "src/lmmodel.h"
#include "..\longmanApp\src\appData\lmData.h"
#include "..\longmanApp\src\appData\lmImageDraw.h"
#include "..\longmanApp\src\appData\lmNormalDraw.h"

#include "..\longmanApp\src\appData\lmDrawManage.h"
//（目前）作用:在工作线程中处理EvtTYPE2类型的Event,建立简单的Event缓冲机制;
//
typedef std::function<void(lmYUVInfo&)> normalCallbacfun;
class cmdProcessThread : public QThread
{
	Q_OBJECT

public:
	cmdProcessThread(QObject *parent = 0);
	~cmdProcessThread();
	bool addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle);
	EvtQue &getEvtQue() { return evtue; };
	QMutex& getmutx() {return mutex;};
	QWaitCondition& getCondition() {return condition;};
	normalCallbacfun recoverhandle;
	//const cyuvParam getlastyuvParam() const { return lastyuvParam; };
protected:
	void run() Q_DECL_OVERRIDE;
private:
	void handleCmd(longmanEvt&);
	//EvtTYPE2类型的Event的列队;
	EvtQue evtue;
	CallBackFuncList _commandTable;
	//yuv数据模块;
	lmData dataModel;
	//当前显示的图片模块;
	QImage mImage;	
	//绘制模块,使用了简单的装饰模式,以便后续功能的扩展和叠加;
	//lmImageDrawBase *mImageDraw;
	lmDrawManage mdraw;
	//SHVC码流解析模块;
	//lmParseStreamPro *mparsestream;
	QMutex mutex;
	QWaitCondition condition;
	//void xOpenYUVFile(longmanEvt&,int);
public:
	bool openyuvfile(longmanEvt&);
	bool changeimagepoc(longmanEvt&);
	bool showyuvData(longmanEvt&);
	bool parseLayerFromList(longmanEvt&);
	bool draw(longmanEvt&);
	bool showcuDepth(longmanEvt&);
	bool showBit(longmanEvt&);
private:
	lmYUVInfoList myuvlist;
	lmYUVInfo curyuv;
};
#endif // cmdProcessThread_h__
