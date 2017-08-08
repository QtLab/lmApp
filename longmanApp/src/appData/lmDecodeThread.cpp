#include "lmDecodeThread.h"
#include <time.h>
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
	qInfo() << "waiting decoding...";
	//longmanEvt testmsg(EvtTYPE1);
	//testmsg.setParam("CommandName", "show_message");
	//testmsg.setParam("MsgType", 0);
	//testmsg.setParam("info", "waiting decoding...");
	//testmsg.dispatch();
	bool decodeSuccessed = false;
	//调用解码,不管解码多少层，最大层级解码必须调用，从而获得各层级PS信息;
	decodeSuccessed = mStreamParse.decoderBitstream(bitstream, maxLayerIdx);
	if (decodeSuccessed)
	{
		{
//#if _DEBUG
		//	std::string cachePath = "C:/Users/Administrator/Documents/GitHub/lmApp/cache/";
//#else
			std::string cachePath = "cache/";
//#endif
			lmDecInfo *decinfo = lmDecInfo::getInstanceForChange();
			decinfo->setInfoSoluPath(cachePath, lmParseStreamPro::getDecYUVName());
			decinfo->readDec();
		}
		//解码剩余层选项;
		for (size_t i = 0; i < layerIdxToDec.size(); i++)
		{
			if (layerIdxToDec[i]!= maxLayerIdx)
			{
				mStreamParse.decoderBitstream(bitstream, i);
			}
		}
		//关闭消息窗口;
		longmanEvt testmsg(EvtTYPE1);
		testmsg.setParam("CommandName", "outputMsg");
		testmsg.setParam("isHide", true);
		testmsg.dispatch();
		//发送openyuvfile事件，刷新显示的yuv,打开最高层级;
		const lmDecInfo *constdecinfo = lmDecInfo::getInstanceForReadonly();
		lmPSData msps(lmPSData::getPSTypeInString(paraTYPE::sps));
		constdecinfo->getPS(msps, maxLayerIdx);
		int mf = msps.getValueByName(msps.getParamName(2)).toInt();
		int mw = msps.getValueByName(msps.getParamName(3)).toInt();
		int mh = msps.getValueByName(msps.getParamName(4)).toInt();
		QString yuvpath = QString::fromStdString(constdecinfo->getyuvPath(maxLayerIdx));
		longmanEvt openyuv(EvtTYPE2);
		openyuv.setParam("CommandName", "open_yuvfile");
		openyuv.setParam("yuv_filePath", QVariant::fromValue(yuvpath));
		openyuv.setParam("yuv_width", QVariant::fromValue(mw));
		openyuv.setParam("yuv_height", QVariant::fromValue(mh));
		openyuv.setParam("yuv_format", QVariant::fromValue(mf));
		openyuv.setParam("yuv_layer", QVariant::fromValue(maxLayerIdx));
		openyuv.dispatch();
		//发送需要显示的解码层，这个数来自于码流文件配置窗口;
		longmanEvt lmlistview(EvtTYPE1);
		lmlistview.setParam("CommandName", "addressLayer_list");
		lmlistview.setParam("maxLayer", QVariant::fromValue(maxLayerIdx));
		lmlistview.setParam("decLayer", QVariant::fromValue(layerDec));
		lmlistview.dispatch();
		return decodeSuccessed;
	}
	else
	{
		qWarning() << "decoding failed";
		//longmanEvt testmsg(EvtTYPE1);
		//testmsg.setParam("CommandName", "show_message");
		//testmsg.setParam("MsgType", 2);
		//testmsg.setParam("info", "decoding failed");
		//testmsg.dispatch();
		return decodeSuccessed;
	}
	
}

bool lmDecodeThread::preDec(longmanEvt& rEvt)
{
	lmParseStreamPro mStreamParse;
	qDebug() << "PreDecoding...";
	std::string bitstream = rEvt.getParam("bitstream_path").toString().toStdString();
	//预解码,获得最大编码层;
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
		qWarning() << "pre-decoding failed";
		//longmanEvt testmsg(EvtTYPE1);
		//testmsg.setParam("CommandName", "show_message");
		//testmsg.setParam("MsgType", 2);
		//testmsg.setParam("info", "pre-decoding failed");
		//testmsg.dispatch();
		return false;
	}
	{
//#if _DEBUG
		//std::string cachePath = "C:/Users/Administrator/Documents/GitHub/lmApp/cache/";
//#else
		std::string cachePath = "cache/";
//#endif
		lmDecInfo *decinfo = lmDecInfo::getInstanceForChange();
		decinfo->setInfoSoluPath(cachePath);
		decinfo->readDec(true);
	}
	//判断预解码是否成功;
	const lmDecInfo *constdecinfo = lmDecInfo::getInstanceForReadonly();
	if (constdecinfo->preDecFailed())
	{
		qWarning() << "readPreDecinfo failed";
		//longmanEvt testmsg(EvtTYPE1);
		//testmsg.setParam("CommandName", "show_message");
		//testmsg.setParam("MsgType", 2);
		//testmsg.setParam("info", "readPreDecinfo failed");
		//testmsg.dispatch();
		return false;
	}
	lmPSData mvps(lmPSData::getPSTypeInString(paraTYPE::vps));
	constdecinfo->getPS(mvps, 0, true);
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
		qInfo() << QStringLiteral("正在解码！");
		//longmanEvt testmsg(EvtTYPE1);
		//testmsg.setParam("CommandName", "show_message");
		//testmsg.setParam("MsgType", 1);
		//testmsg.setParam("info", QStringLiteral("正在解码！"));
		//testmsg.dispatch();
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
	double dResult;
	clock_t lBefore = clock();
	handleCmd(*pEvt);
	dResult = (double)(clock() - lBefore);
	QString mstrmsg = "Command named <" + pEvt->getParam("CommandName").toString() + ">";
	mstrmsg += " spends " + QString::fromStdString(std::to_string(int(dResult))) + " ms,this in Dedcode Thread!";
	qDebug() << mstrmsg;
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
		qCritical() << QStringLiteral("Event的参数列表设置错误！");
// 		longmanEvt testmsg(EvtTYPE1);
// 		testmsg.setParam("CommandName", "show_message");
// 		testmsg.setParam("MsgType", 1);
// 		testmsg.setParam("info", QStringLiteral("Event的参数列表设置错误！"));
// 		testmsg.dispatch();
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
	openyuv.setParam("yuv_filePath", "../cache/rec_layer1.yuv");
	openyuv.setParam("yuv_width", 832);
	openyuv.setParam("yuv_height", 480);
	openyuv.setParam("yuv_format", 1);
	openyuv.dispatch();
}

void lmDecodeThread::xParsePreDecinfo()
{

}
