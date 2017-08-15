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
#include "..\lmDecInfoData\src\lmDecInfo.h"
#include "..\longmanApp\src\appData\lmDrawManage.h"
//��Ŀǰ������:�ڹ����߳��д���EvtTYPE2���͵�Event,�����򵥵�Event�������;
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
	//EvtTYPE2���͵�Event���ж�;
	EvtQue evtue;
	CallBackFuncList _commandTable;
	//yuv����ģ��;
	lmData dataModel;
	//��ǰ��ʾ��ͼƬģ��;
	QImage mImage;	
	//����ģ��,ʹ���˼򵥵�װ��ģʽ,�Ա�������ܵ���չ�͵���;
	lmImageDrawBase *mImageDraw;
	lmDrawManage mdraw;
	//SHVC��������ģ��;
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
private:
	lmYUVInfoList myuvlist;
	lmYUVInfo curyuv;
};
#endif // cmdProcessThread_h__
