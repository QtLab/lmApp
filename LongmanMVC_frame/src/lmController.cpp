#include "lmController.h"
lmController* lmController::_mInstance = nullptr;
//命令构造说明(命令均发往工作线程的列队进行处理);
//1.("CommandName",cmdtable[0])+("yuv_filePath",std::string)+("yuv_width",int)+("yuv_height",int)+("yuv_format",ft)
//2.("CommandName",cmdtable[1])+("yuv_POC",int)+("force_readData", bool)+("recoverLast", bool);
const char *cmdtable[] =
{
	"open_yuvfile",//+文件路径+宽+高+格式(格式转换规则见lmData.cpp);
	"change_imagepoc",//+目标POC+是否强制读取数据(针对固定窗口数据存储方式)+是否恢复上次文件关联(针对已打开文件后的打开失败场景);
	"show_yuvdata",//请在整个工程查找该命令，以获取其参数;
	"parse_shvcbitstream"//解码SHVC码流
};
////绑定命令和相应的处理函数,增加命令时必须添加代码;
void lmController::xcmdInti()
{
	CallBackFunc pcCmdHandle0 = std::bind(&cmdProcessThread::openyuvfile, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[0], pcCmdHandle0);
	CallBackFunc pcCmdHandle1 = std::bind(&cmdProcessThread::changeimagepoc, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[1], pcCmdHandle1);
	CallBackFunc pcCmdHandle2 = std::bind(&cmdProcessThread::showyuvData, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[2], pcCmdHandle2);
	CallBackFunc pcCmdHandle3 = std::bind(&cmdProcessThread::parseSHVCBitBtream, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[3], pcCmdHandle3);
	//callController msgfromworkthread= std::bind(&lmController::sendMsg, this, std::placeholders::_1);
	//workThread.setMsgSendHandle(msgfromworkthread);
	workThread.setParseEXE(&mStreamParse);
}

bool lmController::sendMsg(const std::string& iEvtInfo)
{
	int EvtType = 0;
	workThreadMsg.setParam("CommandName", "show_message");
	workThreadMsg.setParam("MsgType", 0);
	workThreadMsg.setParam("info", QStringLiteral("待处理命令过多,请等待..."));
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
//将EvtTYPE2事件推入事件列队,唤醒工作线程进行处理;
bool lmController::handlevt(longmanEvt& pEvt)
{
	EvtQue &evtue = workThread.getEvtQue();
	if (evtue.size() >= maxQue)
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 0);
		//std::string infoEvtQue = "too many command, "+std::to_string(evtue.size())+" commands left to handle!";
		std::string infoEvtQue = "操作过快！/n还有 " + std::to_string(evtue.size()) + " 个事件等待处理!";
		testmsg.setParam("info", QStringLiteral("对不起，处理过慢，排队事件超过20个！\n***************请等待！****************"));
		testmsg.dispatch();
		EvtQueFull = true;
		return false;//排队事件过多;
	}
	QMutexLocker locker(&workThread.mutex);
	evtue.push_back(pEvt.clone());
	if (!workThread.isRunning())
		workThread.start();
	else
		workThread.condition.wakeOne();
	return true;
}
//将lmController的派生类的回调函数加入列表，类似于model的subscribeEvt函数
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
//通知工作线程重新读取上次成功的yuv文件;
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
