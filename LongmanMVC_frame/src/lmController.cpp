#include "lmController.h"
lmController* lmController::_mInstance = nullptr;
//�����˵��(��������������̵߳��жӽ��д���);
//1.("CommandName",cmdtable[0])+("yuv_filePath",std::string)+("yuv_width",int)+("yuv_height",int)+("yuv_format",ft)
//2.("CommandName",cmdtable[1])+("yuv_POC",int)+("force_readData", bool)+("recoverLast", bool);
const char *cmdtable[] =
{
	"open_yuvfile",//+�ļ�·��+��+��+��ʽ(��ʽת�������lmData.cpp);
	"change_imagepoc",//+Ŀ��POC+�Ƿ�ǿ�ƶ�ȡ����(��Թ̶��������ݴ洢��ʽ)+�Ƿ�ָ��ϴ��ļ�����(����Ѵ��ļ���Ĵ�ʧ�ܳ���);
	"show_yuvdata",//�����������̲��Ҹ�����Ի�ȡ�����;
	"parse_shvcbitstream",//����SHVC����
	"preDecode",//Ԥ����;
	"parse_LayerToshow",
	"draw",//����ģ�飬+"Image"+
	"show_cuDepth",
	"show_bit"
};
////���������Ӧ�Ĵ�����,��������ʱ������Ӵ���;
void lmController::xcmdInti()
{
	//��ͨ�¼�����workThreadִ��;
	//open_yuvfile �¼�������workThread.openyuvfile����
	CallBackFunc pcCmdHandle0 = std::bind(&cmdProcessThread::openyuvfile, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[0], pcCmdHandle0);
	//change_imagepoc �¼�������workThread.changeimagepoc����
	CallBackFunc pcCmdHandle1 = std::bind(&cmdProcessThread::changeimagepoc, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[1], pcCmdHandle1);
	//show_yuvdata �¼�������workThread.changeimagepoc����
	CallBackFunc pcCmdHandle2 = std::bind(&cmdProcessThread::showyuvData, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[2], pcCmdHandle2);
	//�ص�DecodeThread�������̣���ͳһ����workThread,workThread�ص���ģ���callDecodeTread()
	//callDecodeTread()��Evt�ٽ���DecodeThread����DecodeThreadִ��;
	//parse_shvcbitstream �¼���workThreadת����ģ��callDecodeTread()��������pDecoder;
	CallBackFunc pcCmdHandle3 = std::bind(&lmController::callDecodeTread, this, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[3], pcCmdHandle3);
	CallBackFunc pcCmdHandle3_dethread = std::bind(&lmDecodeThread::parseSHVCBitBtream, &pDecoder, std::placeholders::_1);
	pDecoder.addCommandHandle(cmdtable[3], pcCmdHandle3_dethread);
	//preDecode�¼���workThreadת����ģ��callDecodeTread()��������pDecoder;
	CallBackFunc pcCmdHandle4 = std::bind(&lmController::callDecodeTread, this, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[4], pcCmdHandle4);
	CallBackFunc pcCmdHandle4_dethread = std::bind(&lmDecodeThread::preDec, &pDecoder, std::placeholders::_1);
	pDecoder.addCommandHandle(cmdtable[4], pcCmdHandle4_dethread);
	//��ͨ�¼�
	//parse_LayerToshow�¼���ֱ�ӽ���workThread��parseLayerFromList��������;
	CallBackFunc pcCmdHandle5 = std::bind(&cmdProcessThread::parseLayerFromList, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[5], pcCmdHandle5);
	//draw��ֱ�ӽ���workThread��draw��������;
	CallBackFunc pcCmdHandle6 = std::bind(&cmdProcessThread::draw, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[6], pcCmdHandle6);
	//show_cuDepth��ֱ�ӽ���workThread��showcuDepth��������;
	CallBackFunc pcCmdHandle7 = std::bind(&cmdProcessThread::showcuDepth, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[7], pcCmdHandle7);

	//show_bit��ֱ�ӽ���workThread��showBit��������;
	CallBackFunc pcCmdHandle8 = std::bind(&cmdProcessThread::showBit, &workThread, std::placeholders::_1);
	workThread.addCommandHandle(cmdtable[8], pcCmdHandle8);
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
	////Ԥ�����¼�������parse_shvcbitstream�¼�;
	if (pEvt.getParam("CommandName") == "parse_shvcbitstream"
		|| pEvt.getParam("CommandName") == "preDecode")
		pDecoder.start();
		////���빤���߳��¼��б�;
	//else
	//{

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
		QMutexLocker locker(&workThread.getmutx());
		evtue.push_back(pEvt.clone());
	
	if (!workThread.isRunning())
		workThread.start();
	else
		workThread.getCondition().wakeOne();
	/*}*/
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
// 	std::cout << "����SHVC����!" << std::endl;
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
