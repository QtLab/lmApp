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
	//std::cout << "����SHVC����!" << std::endl;
	std::string bitstream = rEvt.getParam("bitstream_path").toString().toStdString();
	int layerDec = rEvt.getParam("numLayerToDec").toInt();
	int maxLayerIdx = rEvt.getParam("maxLayerIdx").toInt();
	layerDec &= 0x0f;
	std::vector<int> layerIdxToDec;
	int t = 0;
	for (size_t i = 0; i < 8; i++)
	{
		t = (layerDec >> i) & 0x01;
		if (t == 1)
			layerIdxToDec.push_back(i);

	}
	longmanEvt testmsg(EvtTYPE1);
	testmsg.setParam("CommandName", "show_message");
	testmsg.setParam("MsgType", 0);
	testmsg.setParam("info", "waiting decoding...");
	testmsg.dispatch();
	bool decodeSuccessed = false;
	//���ý���,���ܽ�����ٲ㣬���㼶���������ã��Ӷ���ø��㼶PS��Ϣ;
	decodeSuccessed = mStreamParse.decoderBitstream(bitstream, maxLayerIdx);
	if (decodeSuccessed)
	{
		decinfo.setInfoSoluPath("C:\\Users\\Administrator\\Documents\\GitHub\\lmApp\\cache\\");
		decinfo.readDec();
		//����ʣ���ѡ��;
		for (size_t i = 0; i < layerIdxToDec.size(); i++)
		{
			if (layerIdxToDec[i]!= maxLayerIdx)
			{
				mStreamParse.decoderBitstream(bitstream, i);
			}
		}
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("isHide", true);
		testmsg.dispatch();
		//����openyuvfile�¼���ˢ����ʾ��yuv,����߲㼶;
		lmPSData msps(lmPSData::getPSTypeInString(paraTYPE::sps));
		decinfo.getPS(msps, maxLayerIdx);
		int mf = msps.getValueByName(msps.getParamName(2)).toInt();
		int mw = msps.getValueByName(msps.getParamName(3)).toInt();
		int mh = msps.getValueByName(msps.getParamName(4)).toInt();
		QString yuvpath = QString::fromStdString( decinfo.retSoluPath() + lmParseStreamPro::getDecYUVName(maxLayerIdx));
		longmanEvt openyuv(EvtTYPE2);
		openyuv.setParam("CommandName", "open_yuvfile");
		openyuv.setParam("yuv_filePath", QVariant::fromValue(yuvpath));
		openyuv.setParam("yuv_width", QVariant::fromValue(mw));
		openyuv.setParam("yuv_height", QVariant::fromValue(mh));
		openyuv.setParam("yuv_format", QVariant::fromValue(mf));
		openyuv.dispatch();
		//������������list;
		longmanEvt lmlistview(EvtTYPE1);
		lmlistview.setParam("CommandName", "addressLayer_list");
		lmlistview.setParam("maxLayer", QVariant::fromValue(maxLayerIdx));
		lmlistview.dispatch();
		return decodeSuccessed;
	}
	else
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 2);
		testmsg.setParam("info", "decoding failed");
		testmsg.dispatch();
		return decodeSuccessed;
	}
	
}

bool lmDecodeThread::preDec(longmanEvt& rEvt)
{
	lmParseStreamPro mStreamParse;
	//std::cout << "����SHVC����!" << std::endl;
	std::string bitstream = rEvt.getParam("bitstream_path").toString().toStdString();
	//Ԥ����,����������;
	bool bPreDecSuccessed = mStreamParse.preDec(bitstream);
	if (!bPreDecSuccessed)
	//{
	//	longmanEvt testmsg(EvtTYPE1);
	//	testmsg.setParam("CommandName", "show_message");
	//	testmsg.setParam("isHide", true);
	//	testmsg.dispatch();
	//}
	//else
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 2);
		testmsg.setParam("info", "decoding failed");
		testmsg.dispatch();
		return false;
	}
	decinfo.setInfoSoluPath("C:\\Users\\Administrator\\Documents\\GitHub\\lmApp\\cache\\");
	decinfo.readDec(true);
	//�ж�Ԥ�����Ƿ�ɹ�;
	if (decinfo.preDecFailed())
	{
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "show_message");
		testmsg.setParam("MsgType", 2);
		testmsg.setParam("info", "pre-decoding failed");
		testmsg.dispatch();
		return false;
	}
	lmPSData mvps(lmPSData::getPSTypeInString(paraTYPE::vps));
	decinfo.getPS(mvps, 0, true);
	int maxLayer = mvps.getValueByName(mvps.getParamName(0)).toInt();
	longmanEvt layer(EvtTYPE1);
	layer.setParam("CommandName", "Get_Layer");
	layer.setParam("MaxLayer", maxLayer);
	layer.dispatch();
	return true;
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
		testmsg.setParam("info", QStringLiteral("���ڽ��룡"));
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
		testmsg.setParam("info", QStringLiteral("Event�Ĳ����б����ô���"));
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

void lmDecodeThread::xParsePreDecinfo()
{

}
