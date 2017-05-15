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
//��Ŀǰ������:�ڹ����߳��д���EvtTYPE2���͵�Event,�����򵥵�Event�������;
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
	//EvtTYPE2���͵�Event���ж�;
	EvtQue evtue;
	CallBackFuncList _commandTable;
	//yuv����ģ��;
	lmData dataModel;
	//��ǰ��ʾ��ͼƬģ��;
	QImage mImage;	
	//�ϴγɹ��򿪵�yuv����;
	cyuvParam lastyuvParam;
	//����ģ��,ʹ���˼򵥵�װ��ģʽ,�Ա�������ܵ���չ�͵���;
	lmImageDrawBase *mImageDraw;
	//SHVC��������ģ��;
	//lmParseStreamPro *mparsestream;
public:
	bool openyuvfile(longmanEvt&);
	bool changeimagepoc(longmanEvt&);
	bool showyuvData(longmanEvt&);
	bool parseSHVCBitBtream(longmanEvt&);
};
#endif // cmdProcessThread_h__
