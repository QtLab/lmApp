#include "lmController.h"
lmController* lmController::_mInstance = nullptr;
//�����˵��(��������������̵߳��жӽ��д���);
//1.("CommandName",cmdtable[0])+("yuv_filePath",std::string)+("yuv_width",int)+("yuv_height",int)+("yuv_format",ft)
//2.("CommandName",cmdtable[1])+("yuv_POC",int)+("force_readData", bool)+("recoverLast", bool);
const char *cmdtable[] =
{
	"open_yuvfile",//+�ļ�·��+��+��+��ʽ(��ʽת�������lmData.cpp);
	"change_imagepoc",//+Ŀ��POC+�Ƿ�ǿ�ƶ�ȡ����(��Թ̶��������ݴ洢��ʽ)+�Ƿ�ָ��ϴ��ļ�����(����Ѵ��ļ���Ĵ�ʧ�ܳ���);
	"show_yuvdata"
};
////���������Ӧ�Ĵ�����,��������ʱ������Ӵ���;
void lmController::xcmdInti()
{
	CallBackFunc pcCmdHandle0 = std::bind(&cmdProcessThread::openyuvfile, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[0], pcCmdHandle0);
	CallBackFunc pcCmdHandle1 = std::bind(&cmdProcessThread::changeimagepoc, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[1], pcCmdHandle1);
	CallBackFunc pcCmdHandle2 = std::bind(&cmdProcessThread::showyuvData, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[2], pcCmdHandle2);
	//callController msgfromworkthread= std::bind(&lmController::sendMsg, this, std::placeholders::_1);
	//workThread.setMsgSendHandle(msgfromworkthread);

}

bool lmController::sendMsg(const std::string& iEvtInfo)
{
	int EvtType = 0;
	workThreadMsg.setParam("CommandName", "show_message");
	workThreadMsg.setParam("MsgType", 0);
	workThreadMsg.setParam("info", QStringLiteral("�������������,��ȴ�..."));
	workThreadMsg.dispatch();
	return true;
}

lmController::lmController(lmEvtBus *evtBus):lmmodel(evtBus),
workThreadMsg(EvtTYPE1)
{
	//setModelName();
	CallBackFunc callbackfunc = std::bind(&lmController::handlevt, this, std::placeholders::_1);
	subscribeEvt(EvtTYPE2, callbackfunc);
	workThread.recoverhandle= std::bind(&lmController::recoverhandle, this, std::placeholders::_1);
	maxQue = 20;
	EvtQueFull = false;
	setModelName("Controller_Model");
	xcmdInti();
	workThread.start();
}

lmController::~lmController()
{
}
//��EvtTYPE2�¼������¼��ж�,���ѹ����߳̽��д���;
bool lmController::handlevt(longmanEvt& pEvt)
{
	EvtQue &evtue = workThread.getEvtQue();
	if (evtue.size() >= maxQue)
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 0);
		//std::string infoEvtQue = "too many command, "+std::to_string(evtue.size())+" commands left to handle!";
		std::string infoEvtQue = "�������죡/n���� " + std::to_string(evtue.size()) + " ���¼��ȴ�����!";
		testmsg.setParam("info", QStringLiteral("�Բ��𣬴���������Ŷ��¼�����20����\n***************��ȴ���****************"));
		testmsg.dispatch();
		EvtQueFull = true;
		return false;//�Ŷ��¼�����;
	}
	QMutexLocker locker(&workThread.mutex);
	evtue.push_back(pEvt.clone());
	if (!workThread.isRunning())
		workThread.start();
	else
		workThread.condition.wakeOne();
	return true;
}
//��lmController��������Ļص����������б�������model��subscribeEvt����
void lmController::registerCommand(const std::string & rpCmdName, const CallBackFunc& rcallbakfunc)
{
	mCallBacklist.insert(CallBackFuncList::value_type(rpCmdName, rcallbakfunc));
}

lmController* lmController::getInstance()
{
	if (_mInstance==nullptr)
		_mInstance= new lmController(nullptr);
	return _mInstance;
}
//֪ͨ�����߳����¶�ȡ�ϴγɹ���yuv�ļ�;
void lmController::recoverhandle(cyuvParam& yuvParam)
{
	//cyuvParam yuvParam = workThread.getlastyuvParam();
	QString filepath = QString::fromStdString(yuvParam.yuvPath);
	longmanEvt openyuv(EvtTYPE2);
	openyuv.setParam("CommandName", "open_yuvfile");
	openyuv.setParam("yuv_filePath", filepath);
	openyuv.setParam("yuv_width", QVariant::fromValue(yuvParam.mWidth));
	openyuv.setParam("yuv_height", QVariant::fromValue(yuvParam.mHeight));
	openyuv.setParam("yuv_format", QVariant::fromValue(yuvParam.mFormat));
	openyuv.dispatch();
}
