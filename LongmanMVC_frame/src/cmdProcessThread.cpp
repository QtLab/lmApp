#include "cmdProcessThread.h"
#include <iostream>
#include <time.h>
	static bool enableShowYuvData = false;
	static bool firstrunChangePOC = true;
cmdProcessThread::cmdProcessThread(QObject *parent):QThread(parent)
{
	//被装饰者实例化;
	mImageDraw = new lmImageDraw;
	//mparsestream = new lmParseStreamPro(this);
}
cmdProcessThread::~cmdProcessThread()
{
/*	if (currentimage)*/
// 	{
// 		delete currentimage;
// 	}
// 	currentimage = nullptr;
}

//bool cmdProcessThread::pushcmd(longmanEvt* cEvt)
//{
//	QMutexLocker locker(&mutex);
//	if (evtue.size() > maxQue)
//		{
//			msgSend("waiting...");
//			EvtQueFull = true;
//			return false;//排队事件过多;
//		}
//	evtue.push_back(cEvt);
//	if (!isRunning()) 
//		start();
//	else 
//		condition.wakeOne();
//	return true;
//}

bool cmdProcessThread::addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle)
{
	CallBackFuncList::iterator miter;
	miter = _commandTable.find(rpCmdName);
	if (miter!=_commandTable.end())
		{
			longmanEvt testmsg(EvtTYPE1);
			testmsg.setParam("CommandName", "show_message");
			testmsg.setParam("MsgType", 1);
			testmsg.setParam("info", QStringLiteral("命令已存在！"));
			testmsg.dispatch();
			return false;
		}
	_commandTable.insert(CallBackFuncList::value_type(rpCmdName, pcCmdHandle));
	return true;
}

bool cmdProcessThread::openyuvfile(longmanEvt& rpevt)
{
	std::string filePath= rpevt.getParam("yuv_filePath").toString().toStdString();
	int mWidth = rpevt.getParam("yuv_width").toInt();
	int mHeight = rpevt.getParam("yuv_height").toInt();
	int mformattype = rpevt.getParam("yuv_format").toInt();
	//dataModel.close();
	int openSt=dataModel.openyuv_r(filePath, false, mformattype, mWidth, mHeight);
	if (openSt<0)
	{
		longmanEvt openstyuv(EvtTYPE1);
		openstyuv.setParam("CommandName", "show_message");
		openstyuv.setParam("MsgType", 1);
		switch (openSt)
		{
		case -1:openstyuv.setParam("info", QStringLiteral("Cannot write a yuv file of bit depth greater than 16 , output will be right-shifted down to 16-bit precision!")); break;
		case -2:openstyuv.setParam("info", QStringLiteral("Cannot read a yuv file of bit depth greater than 16!")); break;
		case -3:openstyuv.setParam("info", QStringLiteral("failed to write output YUV file!")); break;
		case -4:openstyuv.setParam("info", QStringLiteral("failed to open Input YUV file!")); break;
		case -5:openstyuv.setParam("info", QStringLiteral("Sorry,Not enough data in this file!\nReturn last file！")); break;
		case -6:openstyuv.setParam("info", QStringLiteral("Wrong format!")); break;
		
		default:
			break;
		}
		openstyuv.dispatch();
		longmanEvt openstyuvtoMainW(EvtTYPE1);
		openstyuvtoMainW.dispatch();
		return false;
	}
	//保存成功打开的yuv信息;
	lastyuvParam.mFormat = dataModel.getformat();
	lastyuvParam.mWidth = dataModel.getimageWidth();
	lastyuvParam.mHeight = dataModel.getimageHeight();
	lastyuvParam.yuvPath = filePath;
	//给datamodel中的图片存储空间套上QImage类头;
	const unsigned char* imageptr = dataModel.getcurrimage();
	auto rgbformat = lastyuvParam.mFormat? QImage::Format_RGB888 : QImage::Format_Grayscale8;
	mImage = QImage(dataModel.getcurrimage(), lastyuvParam.mWidth, lastyuvParam.mHeight, rgbformat);
	//通知主窗口类(longmanAPP);
	longmanEvt updatemainwin(EvtTYPE1);
	updatemainwin.setParam("CommandName", "update_mainwindow");
	updatemainwin.setParam("yuv_currentPOC", lastyuvParam.mcurPOC);
	updatemainwin.setParam("yuv_totalFrames", dataModel.getTotalFrames());
	updatemainwin.setParam("yuv_width", dataModel.getimageWidth());
	updatemainwin.setParam("yuv_height", dataModel.getimageHeight());
	updatemainwin.setParam("yuv_format", dataModel.getformat());
	updatemainwin.setParam("image", QVariant::fromValue((void*)(&mImage)));
	updatemainwin.dispatch();
	//通知显示模块
	longmanEvt lmgraphview(EvtTYPE1);
	lmgraphview.setParam("CommandName", "set_image");
	lmgraphview.setParam("width", dataModel.getimageWidth());
	lmgraphview.setParam("height", dataModel.getimageHeight());
	lmgraphview.setParam("format", dataModel.getformat());
	lmgraphview.dispatch();
	return true;
}

bool cmdProcessThread::changeimagepoc(longmanEvt& rpevt)
{

	bool openst = rpevt.getParam("recoverLast").toBool();
	//打开文件失败的回退处理;
	if (openst)
		{
			recoverhandle(lastyuvParam);
			return false;
		}
	int mpoc=rpevt.getParam("yuv_POC").toInt();
	bool isforcereadfata = rpevt.getParam("force_readData").toBool();
	if (mpoc==-1)
		return false;
	dataModel.setPOC(mpoc, isforcereadfata);
	dataModel.readPic();
	//**转换为适合显示的PixMap类**;
	//图片内容更新，通知图片显示类;
	QPixmap *mpixmap = mImageDraw->lmDraw(mImage);
	longmanEvt lmgraphview(EvtTYPE1);
 	lmgraphview.setParam("CommandName", "update_image");
	lmgraphview.setParam("Image", QVariant::fromValue((void*)(mpixmap)));
	lmgraphview.dispatch();
	//存储当前POC;
		lastyuvParam.mcurPOC = mpoc;
	

// 	if (enableShowYuvData|| firstrunChangePOC)
// 	{
// 		firstrunChangePOC = !firstrunChangePOC;
// 		//通知yuv数据显示类(lmDataView);
// 		Pel *mYptr = nullptr; Pel *mUptr = nullptr; Pel *mVptr = nullptr;
// 		dataModel.getyuvPtr(mYptr, mUptr, mVptr);
// 		longmanEvt lmdatahview(EvtTYPE1);
// 		lmdatahview.setParam("CommandName", "set_dataview");
// 		lmdatahview.setParam("y_ptr", QVariant::fromValue((void*)(mYptr)));
// 		lmdatahview.setParam("u_ptr", QVariant::fromValue((void*)(mUptr)));
// 		lmdatahview.setParam("v_ptr", QVariant::fromValue((void*)(mVptr)));
// 		lmdatahview.setParam("width", dataModel.getimageWidth());
// 		lmdatahview.setParam("height", dataModel.getimageHeight());
// 		lmdatahview.setParam("format", dataModel.getformat());
// 		lmdatahview.dispatch();
// 	}
	return true;
}

bool cmdProcessThread::showyuvData(longmanEvt& rEvt)
{
	bool showdataEnable = rEvt.getParam("enabledByButton").toBool();
	bool fromGraphView = rEvt.getParam("clickedByGraphView").toBool();
	if (showdataEnable)
		{
			//通知yuv数据显示类(lmDataView);
			if (!enableShowYuvData)
			{
				Pel *mYptr = nullptr; Pel *mUptr = nullptr; Pel *mVptr = nullptr;
				dataModel.getyuvPtr(mYptr, mUptr, mVptr);
				longmanEvt lmdatahview(EvtTYPE1);
				lmdatahview.setParam("CommandName", "set_dataview");
				lmdatahview.setParam("y_ptr", QVariant::fromValue((void*)(mYptr)));
				lmdatahview.setParam("u_ptr", QVariant::fromValue((void*)(mUptr)));
				lmdatahview.setParam("v_ptr", QVariant::fromValue((void*)(mVptr)));
				lmdatahview.setParam("width", dataModel.getimageWidth());
				lmdatahview.setParam("height", dataModel.getimageHeight());
				lmdatahview.setParam("format", dataModel.getformat());
				lmdatahview.dispatch();
			}
			enableShowYuvData = !enableShowYuvData;
		}
	if (!enableShowYuvData)
		{
			if (fromGraphView)
				return true;
			delete mImageDraw;
			mImageDraw = new lmImageDraw;
			QPixmap *mpixmap = mImageDraw->lmDraw(mImage);
			longmanEvt lmgraphview(EvtTYPE1);
			lmgraphview.setParam("CommandName", "update_image");
			lmgraphview.setParam("Image", QVariant::fromValue((void*)(mpixmap)));
			lmgraphview.dispatch();
			return true;
		}
	//删除上次的绘制结果;
	delete mImageDraw;
	mImageDraw = new lmImageDraw;
	int iXmouse = rEvt.getParam("x").toInt();
	int iYmouse = rEvt.getParam("y").toInt();
	mImageDraw = new lmNormalDraw(mImageDraw, iXmouse, iYmouse, dataModel.getimageWidth(), dataModel.getimageHeight());
	//通知图片显示类;
	QPixmap *mpixmap = mImageDraw->lmDraw(mImage);

	longmanEvt lmgraphview(EvtTYPE1);
	lmgraphview.setParam("CommandName", "update_image");
	lmgraphview.setParam("Image", QVariant::fromValue((void*)(mpixmap)));
	lmgraphview.dispatch();
	//通知dataview模块;
	longmanEvt dataview(EvtTYPE1);
	dataview.setParam("CommandName", "update_dataview");
	dataview.setParam("xIn16", iXmouse);
	dataview.setParam("yIn16", iYmouse);
	dataview.dispatch();
	return true;
}

bool cmdProcessThread::parseLayerFromList(longmanEvt& rEvt)
{
	int layerIdx = rEvt.getParam("layerIdx").toInt();
	const lmDecInfo* cDec = lmDecInfo::getInstanceForReadonly();
	lmPSData msps(lmPSData::getPSTypeInString(paraTYPE::sps));
	cDec->getPS(msps, layerIdx);
	int mf = msps.getValueByName(msps.getParamName(2)).toInt();
	int mw = msps.getValueByName(msps.getParamName(3)).toInt();
	int mh = msps.getValueByName(msps.getParamName(4)).toInt();
	QString yuvpath = QString::fromStdString(cDec->getyuvPath(layerIdx));
	longmanEvt openyuv(EvtTYPE2);
	openyuv.setParam("CommandName", "open_yuvfile");
	openyuv.setParam("yuv_filePath", QVariant::fromValue(yuvpath));
	openyuv.setParam("yuv_width", QVariant::fromValue(mw));
	openyuv.setParam("yuv_height", QVariant::fromValue(mh));
	openyuv.setParam("yuv_format", QVariant::fromValue(mf));
	openyuvfile(openyuv);
	//直接调用openyuvfile函数
	return true;
}

// bool cmdProcessThread::parseSHVCBitBtream(longmanEvt& rEvt)
// {
// 	std::cout << "解析SHVC码流!" << std::endl;
// 	std::string bitstream = rEvt.getParam("bitstream_path").toString().toStdString();
// 	int layerNum = rEvt.getParam("layer_num").toInt();
// 	bool decodesuccessed = false;
// 	decodesuccessed= mStreamParse->decoderBitstream(bitstream, layerNum);
// 	return true;
// }

void cmdProcessThread::handleCmd(longmanEvt& requstCmd)
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

void cmdProcessThread::xOpenYUVFile(longmanEvt&)
{

}

void cmdProcessThread::run()
{
	forever
	{
	mutex.lock();
	if (evtue.empty())
		condition.wait(&mutex);
	Double dResult;
	clock_t lBefore = clock();
	longmanEvt *pEvt = evtue.front();
	evtue.pop_front();
	mutex.unlock();
	handleCmd(*pEvt);
	dResult = (Double)(clock() - lBefore) / CLOCKS_PER_SEC;
	if(pEvt->getEvtTYPE()==EvtTYPE2)
	std::cout << "EXE处理时间："<<dResult <<"s" << std::endl;
	delete pEvt;
	}
}