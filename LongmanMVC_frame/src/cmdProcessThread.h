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
#include "..\longmanApp\src\appData\lmParseStreamPro.h"
#include "..\longmanApp\src\appData\lmImageDraw.h"
#include "..\longmanApp\src\appData\lmNormalDraw.h"
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
class cmdProcessThread : public QThread
{
	Q_OBJECT

public:
	cmdProcessThread(QObject *parent = 0);
	~cmdProcessThread();
	bool addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle);
	EvtQue &getEvtQue() { return evtue; };
	QMutex mutex;
	QWaitCondition condition;
	normalCallbacfun recoverhandle;
	const cyuvParam getlastyuvParam() const { return lastyuvParam; };
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
	//上次成功打开的yuv参数;
	cyuvParam lastyuvParam;
	//绘制模块,使用了简单的装饰模式,以便后续功能的扩展和叠加;
	lmImageDrawBase *mImageDraw;
	//SHVC码流解析模块;
	//lmParseStreamPro *mparsestream;
public:
	bool openyuvfile(longmanEvt&);
	bool changeimagepoc(longmanEvt&);
	bool showyuvData(longmanEvt&);
	bool parseSHVCBitBtream(longmanEvt&);
};
#endif // cmdProcessThread_h__
