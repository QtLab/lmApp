#include "lmDecodeThread.h"
//using std::cout;
lmDecodeThread::lmDecodeThread(QObject *parent)
	: QThread(parent)
{
}

lmDecodeThread::~lmDecodeThread()
{
}

bool lmDecodeThread::parseSHVCBitBtream(longmanEvt& rEvt)
{
	lmParseStreamPro mStreamParse;
	//std::cout << "解析SHVC码流!" << std::endl;
	std::string bitstream = rEvt.getParam("bitstream_path").toString().toStdString();
	int layerNum = rEvt.getParam("layer_num").toInt();
	longmanEvt testmsg(EvtTYPE1);
	testmsg.setParam("CommandName", "show_message");
	testmsg.setParam("MsgType", 0);
	testmsg.setParam("info", "waiting decoding...");
	testmsg.dispatch();
	bool decodeSuccessed = false;
	decodeSuccessed = mStreamParse.decoderBitstream(bitstream, layerNum);
	if (decodeSuccessed)
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("isHide", true);
		testmsg.dispatch();
		xParseinfo();

	}
	else
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 2);
		testmsg.setParam("info", "decoding failed");
		testmsg.dispatch();
	}
	return decodeSuccessed;
}

bool lmDecodeThread::addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle)
{
	CallBackFuncList::iterator miter;
	miter = _commandTable.find(rpCmdName);
	if (miter != _commandTable.end())
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 1);
		testmsg.setParam("info", QStringLiteral("正在解码！"));
		testmsg.dispatch();
		return false;
	}
	_commandTable.insert(CallBackFuncList::value_type(rpCmdName, pcCmdHandle));
	return true;
}

void lmDecodeThread::run()
{
	forever
	{
	mutex.lock();
	if (evtue.empty())
		condition.wait(&mutex);
	longmanEvt *pEvt = evtue.front();
	evtue.pop_front();
	mutex.unlock();
	handleCmd(*pEvt);
	delete pEvt;
	}
}

void lmDecodeThread::handleCmd(longmanEvt& requstCmd)
{
	CallBackFuncList::iterator m_cmdHandlef;
	paramlist::const_iterator paramBeg;
	paramlist::const_iterator paramEnd;
	if (!(requstCmd.getParamIter(paramBeg, paramEnd)) || paramBeg->first != "CommandName")
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 1);
		testmsg.setParam("info", QStringLiteral("Event的参数列表设置错误！"));
		testmsg.dispatch();
		return;
	}
	std::string &m_cmdName = (paramBeg->second).toString().toStdString();
	m_cmdHandlef = _commandTable.find(m_cmdName);
	if (m_cmdHandlef != _commandTable.end())
	{
		m_cmdHandlef->second(requstCmd);
	}
	return;
}

void lmDecodeThread::xParseinfo()
{
	longmanEvt openyuv(EvtTYPE2);
	openyuv.setParam("CommandName", "open_yuvfile");
	openyuv.setParam("yuv_filePath", "..\\cache\\rec_layer1.yuv");
	openyuv.setParam("yuv_width", 832);
	openyuv.setParam("yuv_height", 480);
	openyuv.setParam("yuv_format", 1);
	openyuv.dispatch();
}
