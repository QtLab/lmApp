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
//��Ŀǰ������:�ڹ����߳��д���EvtTYPE2���͵�Event,�����򵥵�Event�������;
//
// struct cyuvParam
// {
// 	int mWidth = 0;
// 	int mHeight = 0;
// 	int mFormat = 1;
// 	int mcurPOC = 0;
// 	std::string yuvPath;
// };
typedef std::function<void(lmYUVInfoList&)> normalCallbacfun;
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
	//bool parseSHVCBitBtream(longmanEvt&);
	//void setParseEXE(lmParseStreamPro *pStreamParse) { mStreamParse = pStreamParse; };
//private:
	//lmParseStreamPro *mStreamParse = nullptr;
	//�ɹ��򿪵�yuv��Ϣ;
	lmYUVInfoList myuvlist;
};
#endif // cmdProcessThread_h__
