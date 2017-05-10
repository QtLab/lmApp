#include "cmdProcessThread.h"
#include <iostream>
#include <time.h>
	static bool enableShowYuvData = false;
	static bool firstrunChangePOC = true;
cmdProcessThread::cmdProcessThread(QObject *parent):QThread(parent)
{
	//��װ����ʵ����;
	mImageDraw = new lmImageDraw;
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
//			return false;//�Ŷ��¼�����;
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
			testmsg.setParam("info", QStringLiteral("�����Ѵ��ڣ�"));
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
		case -5:openstyuv.setParam("info", QStringLiteral("Sorry,Not enough data in this file!\nReturn last file��")); break;
		default:
			break;
		}
		openstyuv.dispatch();
		return false;
	}
	//����ɹ��򿪵�yuv��Ϣ;
	lastyuvParam.mFormat = dataModel.getformat();
	lastyuvParam.mWidth = dataModel.getimageWidth();
	lastyuvParam.mHeight = dataModel.getimageHeight();
	lastyuvParam.yuvPath = filePath;
	//��datamodel�е�ͼƬ�洢�ռ�����QImage��ͷ;
	const unsigned char* imageptr = dataModel.getcurrimage();
	auto rgbformat = lastyuvParam.mFormat? QImage::Format_RGB888 : QImage::Format_Grayscale8;
	mImage = QImage(dataModel.getcurrimage(), lastyuvParam.mWidth, lastyuvParam.mHeight, rgbformat);
	//֪ͨ��������(longmanAPP);
	longmanEvt updatemainwin(EvtTYPE1);
	updatemainwin.setParam("CommandName", "update_mainwindow");
	updatemainwin.setParam("yuv_currentPOC", lastyuvParam.mcurPOC);
	updatemainwin.setParam("yuv_totalFrames", dataModel.getTotalFrames());
	updatemainwin.setParam("yuv_width", dataModel.getimageWidth());
	updatemainwin.setParam("yuv_height", dataModel.getimageHeight());
	updatemainwin.setParam("yuv_format", dataModel.getformat());
	updatemainwin.setParam("image", QVariant::fromValue((void*)(&mImage)));
	updatemainwin.dispatch();
	return true;
}

bool cmdProcessThread::changeimagepoc(longmanEvt& rpevt)
{

	bool openst = rpevt.getParam("recoverLast").toBool();
	//���ļ�ʧ�ܵĻ��˴���;
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
	//**ת��Ϊ�ʺ���ʾ��PixMap��**;
	//ͼƬ���ݸ��£�֪ͨͼƬ��ʾ��;
	QPixmap *mpixmap = mImageDraw->lmDraw(mImage);
	longmanEvt lmgraphview(EvtTYPE1);
 	lmgraphview.setParam("CommandName", "update_image");
	lmgraphview.setParam("Image", QVariant::fromValue((void*)(mpixmap)));
	lmgraphview.dispatch();
	//�洢��ǰPOC;
	lastyuvParam.mcurPOC = mpoc;

// 	if (enableShowYuvData|| firstrunChangePOC)
// 	{
// 		firstrunChangePOC = !firstrunChangePOC;
// 		//֪ͨyuv������ʾ��(lmDataView);
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
			//֪ͨyuv������ʾ��(lmDataView);
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
	//ɾ���ϴεĻ��ƽ��;
	delete mImageDraw;
	mImageDraw = new lmImageDraw;
	int iXmouse = rEvt.getParam("x").toInt();
	int iYmouse = rEvt.getParam("y").toInt();
	mImageDraw = new lmNormalDraw(mImageDraw, iXmouse, iYmouse, dataModel.getimageWidth(), dataModel.getimageHeight());
	//֪ͨͼƬ��ʾ��;
	QPixmap *mpixmap = mImageDraw->lmDraw(mImage);
	longmanEvt lmgraphview(EvtTYPE1);
	lmgraphview.setParam("CommandName", "update_image");
	lmgraphview.setParam("Image", QVariant::fromValue((void*)(mpixmap)));
	lmgraphview.dispatch();
	//֪ͨdataviewģ��;
	longmanEvt dataview(EvtTYPE1);
	dataview.setParam("CommandName", "update_dataview");
	dataview.setParam("xIn16", iXmouse);
	dataview.setParam("yIn16", iYmouse);
	dataview.dispatch();
	return true;
}

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
	std::cout << "EXE����ʱ�䣺"<<dResult <<"s" << std::endl;
	delete pEvt;
	}
}