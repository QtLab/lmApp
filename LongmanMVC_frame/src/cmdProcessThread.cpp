#include "cmdProcessThread.h"
#include <time.h>
	static bool enableShowYuvData = false;//
	static bool firstrunChangePOC = true;
cmdProcessThread::cmdProcessThread(QObject *parent):QThread(parent), mImageDraw(new lmImageDraw)
{
}
cmdProcessThread::~cmdProcessThread()
{
}
bool cmdProcessThread::addCommandHandle(const std::string& rpCmdName, CallBackFunc& pcCmdHandle)
{
	CallBackFuncList::iterator miter;
	miter = _commandTable.find(rpCmdName);
	if (miter!=_commandTable.end())
		{
			qWarning() << QStringLiteral("Event�����Ѵ��ڣ�");
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
	int mLayer = rpevt.getParam("yuv_layer").toInt();
	//dataModel.close();
	int openSt=dataModel.openyuv_r(filePath, false, mformattype, mWidth, mHeight);
	if (openSt<0)
	{
		QString in;
		switch (openSt)
		{
		case -1:in = QStringLiteral("Cannot write a yuv file of bit depth greater than 16 , output will be right-shifted down to 16-bit precision!"); break;
		case -2:in = QStringLiteral("Cannot read a yuv file of bit depth greater than 16!"); break;
		case -3:in = QStringLiteral("failed to write output YUV file!"); break;
		case -4:in = QStringLiteral("failed to open Input YUV file!"); break;
		case -5:in =QStringLiteral("Sorry,Not enough data in this file!\nReturn last file��"); break;
		case -6:in = QStringLiteral("Wrong format!"); break;
		default:in = QStringLiteral("something happened");
			break;
		}
		
		qCritical() << in;
		//֪ͨ�����ڣ���ʧ��;
		longmanEvt openstyuvtoMainW(EvtTYPE1);
		openstyuvtoMainW.setParam("CommandName", "fail_openyuv");
		openstyuvtoMainW.dispatch();
		return false;
	}
	//����ɹ��򿪵�yuv��Ϣ;
	lmYUVInfo tyuvinfo(filePath, dataModel.getimageWidth(), dataModel.getimageHeight(), dataModel.getformat());
	tyuvinfo.setLayer(mLayer);
	curyuv = tyuvinfo;
	myuvlist << tyuvinfo;
	//��datamodel�е�ͼƬ�洢�ռ�����QImage��ͷ;
	const unsigned char* imageptr = dataModel.getcurrimage();
	auto rgbformat = tyuvinfo.getFormat()? QImage::Format_RGB888 : QImage::Format_Grayscale8;
	mImage = QImage(dataModel.getcurrimage(), tyuvinfo.getWidth(), tyuvinfo.getHeight(), rgbformat);
	//֪ͨ��������(longmanAPP);
	longmanEvt updatemainwin(EvtTYPE1);
	updatemainwin.setParam("CommandName", "update_mainwindow");
	updatemainwin.setParam("yuv_currentPOC", tyuvinfo.getPoc());
	updatemainwin.setParam("yuv_totalFrames", dataModel.getTotalFrames());
	updatemainwin.setParam("yuv_width", dataModel.getimageWidth());
	updatemainwin.setParam("yuv_height", dataModel.getimageHeight());
	updatemainwin.setParam("yuv_format", dataModel.getformat());
	updatemainwin.setParam("image", QVariant::fromValue((void*)(&mImage)));
	updatemainwin.dispatch();
	//֪ͨ��ʾģ��
// 	longmanEvt lmgraphview(EvtTYPE1);
// 	lmgraphview.setParam("CommandName", "set_image");
// 	lmgraphview.setParam("width", dataModel.getimageWidth());
// 	lmgraphview.setParam("height", dataModel.getimageHeight());
// 	lmgraphview.setParam("format", dataModel.getformat());
// 	lmgraphview.dispatch();
	return true;
}

bool cmdProcessThread::changeimagepoc(longmanEvt& rpevt)
{

	bool openst = rpevt.getParam("recoverLast").toBool();
	//���ļ�ʧ�ܵĻ��˴���;
	if (openst)
		{
			recoverhandle(curyuv);
			return false;
		}
	int mpoc=rpevt.getParam("yuv_POC").toInt();
	bool isforcereadfata = rpevt.getParam("force_readData").toBool();
	if (mpoc==-1)
		return false;
	dataModel.setPOC(mpoc, isforcereadfata);
	dataModel.readPic();
	//ͼƬ���ݸ��£�֪ͨ����ģ��;
	longmanEvt drawImage(EvtTYPE2);
 	drawImage.setParam("CommandName", "draw");
	drawImage.setParam("Image", QVariant::fromValue((void*)(&mImage)));
	drawImage.setParam("drawTypeCode", lmDrawManage::drawType::showImage);
	drawImage.dispatch();
	//�洢��ǰPOC;
	curyuv.setPOC(mpoc);
	return true;
}

bool cmdProcessThread::showyuvData(longmanEvt& rEvt)
{
	bool showdataEnableFromAPPWin = rEvt.getParam("enabledByButton").toBool();
	if (showdataEnableFromAPPWin)
	{
		Pel *mYptr = nullptr; Pel *mUptr = nullptr; Pel *mVptr = nullptr;
		dataModel.getyuvPtr(mYptr, mUptr, mVptr);
		//֪ͨ������ʾ����;
		longmanEvt lmdatahview(EvtTYPE1);
		lmdatahview.setParam("CommandName", "set_dataview");
		lmdatahview.setParam("y_ptr", QVariant::fromValue((void*)(mYptr)));
		lmdatahview.setParam("u_ptr", QVariant::fromValue((void*)(mUptr)));
		lmdatahview.setParam("v_ptr", QVariant::fromValue((void*)(mVptr)));
		lmdatahview.setParam("width", dataModel.getimageWidth());
		lmdatahview.setParam("height", dataModel.getimageHeight());
		lmdatahview.setParam("format", dataModel.getformat());
		lmdatahview.setParam("xIn16", 0);
		lmdatahview.setParam("yIn16", 0);
		lmdatahview.dispatch();
		//֪ͨ����ģ��;
		longmanEvt drawImage(EvtTYPE2);
		drawImage.setParam("CommandName", "draw");
		drawImage.setParam("drawTypeCode", lmDrawManage::drawType::yuvdata);
		drawImage.dispatch();
	}
	else
	{

		longmanEvt drawImage(EvtTYPE2);
		drawImage.setParam("CommandName", "draw");
		drawImage.setParam("drawTypeCode", lmDrawManage::drawType::showImage);
		drawImage.dispatch();
	}
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
	openyuv.setParam("yuv_layer", QVariant::fromValue(layerIdx));
	//ֱ�ӵ���xopenyuvfile����;
	openyuvfile(openyuv);
	return true;
}

bool cmdProcessThread::draw(longmanEvt& rEvt)
{
	return mdraw.handleEvt(rEvt);
}

void cmdProcessThread::handleCmd(longmanEvt& requstCmd)
{
	CallBackFuncList::iterator m_cmdHandlef;
	paramlist::const_iterator paramBeg;
	paramlist::const_iterator paramEnd;
	if (!(requstCmd.getParamIter(paramBeg, paramEnd)) || paramBeg->first != "CommandName")
	{
		qCritical() << QStringLiteral("Event�Ĳ����б����ô���");
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
	dResult = (Double)(clock() - lBefore);
	QString mstrmsg ="Command named <"+ pEvt->getParam("CommandName").toString()+">";
	mstrmsg += " spends "+QString::fromStdString(std::to_string(int(dResult))) + " ms, this is in workThread!";
	qDebug() << mstrmsg;
	delete pEvt;
	}
}