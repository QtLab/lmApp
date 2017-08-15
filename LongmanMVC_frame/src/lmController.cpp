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
	"parse_shvcbitstream",//解码SHVC码流
	"preDecode",//预解码;
	"parse_LayerToshow",
	"draw",//绘制模块，+"Image"+
	"show_cuDepth"
};
////绑定命令和相应的处理函数,增加命令时必须添加代码;
void lmController::xcmdInti()
{
	//普通事件交给workThread执行;
	//open_yuvfile 事件，交给workThread.openyuvfile（）
	CallBackFunc pcCmdHandle0 = std::bind(&cmdProcessThread::openyuvfile, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[0], pcCmdHandle0);
	//change_imagepoc 事件，交给workThread.changeimagepoc（）
	CallBackFunc pcCmdHandle1 = std::bind(&cmdProcessThread::changeimagepoc, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[1], pcCmdHandle1);
	//show_yuvdata 事件，交给workThread.changeimagepoc（）
	CallBackFunc pcCmdHandle2 = std::bind(&cmdProcessThread::showyuvData, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[2], pcCmdHandle2);
	//回调DecodeThread函数过程：先统一交给workThread,workThread回调本模块的callDecodeTread()
	//callDecodeTread()将Evt再交给DecodeThread，由DecodeThread执行;
	//parse_shvcbitstream 事件，workThread转交本模块callDecodeTread()，再推入pDecoder;
	CallBackFunc pcCmdHandle3 = std::bind(&lmController::callDecodeTread, this, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[3], pcCmdHandle3);
	CallBackFunc pcCmdHandle3_dethread = std::bind(&lmDecodeThread::parseSHVCBitBtream, &pDecoder, std::placeholders::_1);
	pDecoder.addCommandHandle(cmdtable[3], pcCmdHandle3_dethread);
	//preDecode事件，workThread转交本模块callDecodeTread()，再推入pDecoder;
	CallBackFunc pcCmdHandle4 = std::bind(&lmController::callDecodeTread, this, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[4], pcCmdHandle4);
	CallBackFunc pcCmdHandle4_dethread = std::bind(&lmDecodeThread::preDec, &pDecoder, std::placeholders::_1);
	pDecoder.addCommandHandle(cmdtable[4], pcCmdHandle4_dethread);
	//普通事件
	//parse_LayerToshow事件，直接交给workThread的parseLayerFromList函数处理;
	CallBackFunc pcCmdHandle5 = std::bind(&cmdProcessThread::parseLayerFromList, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[5], pcCmdHandle5);
	//draw，直接交给workThread的draw函数处理;
	CallBackFunc pcCmdHandle6 = std::bind(&cmdProcessThread::draw, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[6], pcCmdHandle6);
	//show_cuDepth，直接交给workThread的showcuDepth函数处理;
	CallBackFunc pcCmdHandle7 = std::bind(&cmdProcessThread::showcuDepth, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[7], pcCmdHandle7);
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
	////预处理事件：拦截parse_shvcbitstream事件;
	if (pEvt.getParam("CommandName") == "parse_shvcbitstream"
		|| pEvt.getParam("CommandName") == "preDecode")
		pDecoder.start();
		////推入工作线程事件列表;
	//else
	//{

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
		QMutexLocker locker(&workThread.getmutx());
		evtue.push_back(pEvt.clone());
	
	if (!workThread.isRunning())
		workThread.start();
	else
		workThread.getCondition().wakeOne();
	/*}*/
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
void lmController::recoverhandle(lmYUVInfo& yuvParam)
{
	//cyuvParam yuvParam = workThread.getlastyuvParam();
	auto mit = yuvParam;
	QString filepath = QString::fromStdString(mit.absyuvPath());
	longmanEvt openyuv(EvtTYPE2);
	openyuv.setParam("CommandName", "open_yuvfile");
	openyuv.setParam("yuv_filePath", filepath);
	openyuv.setParam("yuv_width", QVariant::fromValue(mit.getWidth()));
	openyuv.setParam("yuv_height", QVariant::fromValue(mit.getHeight()));
	openyuv.setParam("yuv_format", QVariant::fromValue(mit.getFormat()));
	openyuv.setParam("yuv_layer", QVariant::fromValue(mit.getLayer()));
	openyuv.setParam("yuv_decoded", QVariant::fromValue(mit.getdecoded()));
	openyuv.dispatch();
}

bool lmController::callDecodeTread(longmanEvt& rEvt)
{
	EvtQue &evtue = pDecoder.getEvtQue();
	QMutexLocker locker(&pDecoder.getMutx());
	evtue.push_back(rEvt.clone());
	if (!pDecoder.isRunning())
		pDecoder.start();
	else
		pDecoder.getCondition().wakeOne(); 

	return true;
}

// bool lmController::parseSHVCBitBtream(longmanEvt& rEvt)
// {
// 	lmParseStreamPro mStreamParse;
// 	std::cout << "解析SHVC码流!" << std::endl;
// 	std::string bitstream = rEvt.getParam("bitstream_path").toString().toStdString();
// 	int layerNum = rEvt.getParam("layer_num").toInt();
// 	sendMsg("waiting decoding...");
// 	//longmanEvt testmsg(EvtTYPE1);
// 	//testmsg.setParam("CommandName", "show_message");
// 	//testmsg.setParam("MsgType", 0);
// 	//testmsg.setParam("info", "waiting decoding...");
// 	//testmsg.dispatch();
// 	bool decodeSuccessed = false;
// 	decodeSuccessed = mStreamParse.decoderBitstream(bitstream, layerNum);
// 	//if (decodeSuccessed)
// 	//{
// 	//	longmanEvt testmsg(EvtTYPE1);
// 	//	testmsg.setParam("CommandName", "show_message");
// 	//	testmsg.setParam("isHide", true);
// 	//	testmsg.dispatch();
// 	//}
// 	return true;
// }
