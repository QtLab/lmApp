#include "longmanApp.h"
#include<QImageWriter>

//命令构造说明;
//1.("CommandName","update_mainwindow")+("yuv_currentPOC",int)+("yuv_totalFrames",int)+;
//("yuv_width",int)+("yuv_height",int)+("image",uchar*)+;
//一般只在成功打开yuv文件成功后执行一次,用于更新主窗体显示的yuv信息,以及当前显示的图片的指针;
//用于保存为图片;
const char *formattext[] = { "400","420","444" };
static bool openFailed = true;//1：打开失败；0：打开成功;
static int formattype = 1;
longmanApp::longmanApp(QWidget *parent)
	: QMainWindow(parent),
	lmView(nullptr),
	m_imageView(new lmGraphView(this)),
	imageSave(nullptr),
	m_DataView(new lmDataView(this)),
	mBitParseCFG(new lmParserBitConfigure(this)),
	mlayerList(new lmLayerList(this)),
	mMsgOutput(new lmMsgView(this))
{
	timeLine.stop();
	ui.setupUi(this);
	ui.gridLayout_8->addWidget(m_imageView);
	
	ui.gridLayout_6->addWidget(mlayerList,0,0, Qt::AlignTop);
	ui.gridLayout_10->addWidget(mMsgOutput);
	CallBackFunc pcupdate = std::bind(&longmanApp::updatemainwindow, this, std::placeholders::_1);
	listenParam("update_mainwindow", pcupdate);
	CallBackFunc pcupdate1 = std::bind(&longmanApp::openyuvFailed, this, std::placeholders::_1);
	listenParam("fail_openyuv", pcupdate1);
	setModelName("MainWindow_View_Model");
	//
	//connect(ui.FrameIdxSlider, SIGNAL(ValueChanged(int)), this, SLOT(on_FrameIdxSlider_ValueChanged(int)));
	//
	ui.YUVgroupBox->setEnabled(false);
	ui.stopButton->setEnabled(false);
	ui.actionSave_as_image->setEnabled(false);
	ui.f3Button->setEnabled(false);
	ui.f1Button->setEnabled(false);
	connect(&timeLine, SIGNAL(frameChanged(int)), this, SLOT(player(int)));
}

bool longmanApp::updatemainwindow(longmanEvt& updateWinEvt)
{
	openFailed = false;
	ui.YUVgroupBox->setEnabled(true);
	int curpoc = updateWinEvt.getParam("yuv_currentPOC").toInt();
	int totalfarmes= updateWinEvt.getParam("yuv_totalFrames").toInt();
	int widthImage= updateWinEvt.getParam("yuv_width").toInt();
	int heightImage = updateWinEvt.getParam("yuv_height").toInt();
	formattype = updateWinEvt.getParam("yuv_format").toInt();
	QVariant vValue = updateWinEvt.getParam("image");
	imageSave = (QImage*)vValue.value<void *>();
	ui.FrameIdxSlider->setMaximum(totalfarmes-1);

	ui.spinBoxhei->setValue(heightImage);
	ui.spinBoxwid->setValue(widthImage);
	ui.MaxFraBox->setValue(totalfarmes);
	ui.label_format->setText(tr(formattext[formattype]));
	ui.actionSave_as_image->setEnabled(true);
	ui.f3Button->setEnabled(true);
	ui.f1Button->setEnabled(true);
	//修复第二次以后打开没有及时更新内容的问题;
	//curpoc一般为0;
	//解决当Slider=0时，打开调用ui.FrameIdxSlider->setValue(curpoc),无法刷新;
	if (ui.FrameIdxSlider->value() > 0)
		ui.FrameIdxSlider->setValue(curpoc);
	else
		//强制刷新;
		on_FrameIdxSlider_valueChanged(curpoc);
	
	return true;
}

bool longmanApp::openyuvFailed(longmanEvt&)
{
	//ui.FrameIdxSlider->setMinimum(-1);
	//ui.FrameIdxSlider->setValue(-1);
	openFailed = true;
	//强制刷新;
	sendEvttoChnagePOC(0);
	return true;
}

void longmanApp::xMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &strMsg)
{
// 	QByteArray localMsg = strMsg.toLocal8Bit();
// 	switch (type) {
// 	case QtDebugMsg:
// 		fprintf(stdout, "[Debug]: %s\n", localMsg.constData());
// 		break;
// 	case QtWarningMsg:
// 		fprintf(stderr, "[Warn]: %s\n", localMsg.constData());
// 		break;
// 	case QtCriticalMsg:
// 		fprintf(stderr, "[Crit]: %s\n", localMsg.constData());
// 		break;
// 	case QtFatalMsg:
// 		fprintf(stderr, "[Fata]: %s\n", localMsg.constData());
// 	}
// 	fflush(stdout);
// 	fflush(stderr);
	longmanEvt msg(EvtTYPE1);
	msg.setParam("CommandName", "outputMsg");
	msg.setParam("MsgType", (int)type);
	msg.setParam("info", strMsg);
	msg.dispatch();
	if (type == QtFatalMsg)
		abort();
}

void longmanApp::sendEvttoChnagePOC(int ppoc)
{
	//构建图片更新事件;
	//由于不再使用窗口存储图片，force_readData参数总为真;
	int curpoc = ppoc;
	longmanEvt yuvnext(EvtTYPE2);
	yuvnext.setParam("CommandName", "change_imagepoc");
	yuvnext.setParam("yuv_POC", curpoc);
	if (openFailed)//打开失败，回退;
		yuvnext.setParam("recoverLast", true);
	else
		yuvnext.setParam("recoverLast", false);
	yuvnext.setParam("force_readData", true);
	yuvnext.dispatch();
}

void longmanApp::on_actionOpen_SHVC_bitstream_triggered()
{
	QString bspath;
	//int layertobedecode = 1;

#if 1
	mBitParseCFG->resetState();
	if (mBitParseCFG->exec() == QDialog::Accepted&&mBitParseCFG->getcfg(bspath))
		{
			longmanEvt parsestream(EvtTYPE2);
			parsestream.setParam("CommandName", "preDecode");
			parsestream.setParam("bitstream_path", bspath);
			parsestream.dispatch();
		}
#else
	longmanEvt parsestream(EvtTYPE2);
	parsestream.setParam("CommandName", "parse_shvcbitstream");
	parsestream.setParam("bitstream_path", "C:/Users/Administrator/Documents/str.bin");
	parsestream.dispatch();
#endif
}

void longmanApp::on_actionOpen_triggered()
{
	//这里还有BUG，20170810;
// 	if (OpenNum==-1)
// 	{
// 		ui.FrameIdxSlider->setMinimum(-1);
// 		ui.FrameIdxSlider->setValue(-1);
// 	}
	QFileDialog dialog(this, QStringLiteral("open yuv file"));
	if (openFailed) {
		const QString defaultLocations = QDir::currentPath()+"/cache";
		dialog.setDirectory(defaultLocations.isEmpty() ? QDir::currentPath() : defaultLocations);
	}
	dialog.setNameFilter(QStringLiteral("YUV file (*.yuv)"));//文件格式设置;
	QString FliePath; //QString YuvName("v");
	if (dialog.exec() != QDialog::Accepted && dialog.selectedFiles().isEmpty())
		return;
	FliePath = dialog.selectedFiles().first();
	std::string fp = FliePath.toStdString();
	auto w = fp.end();
	char  v = *(--w);
	if (v!= 'v'&& v != 'V')
		return;
	yuvParamSet paramselect(this);
	if (paramselect.exec() == QDialog::Rejected)
		return;
	if (paramselect.getyuvwidth()%2!=0|| paramselect.getyuvheight() % 2 != 0)
	{
		qCritical() << QStringLiteral("尺寸不能为奇数！");
// 		longmanEvt paramWarning(EvtTYPE1);
// 		paramWarning.setParam("CommandName", "show_message");
// 		paramWarning.setParam("MsgType", 2);
// 		paramWarning.setParam("info", QStringLiteral("尺寸不能为奇数！"));
// 		paramWarning.dispatch();
		return;
	}
	longmanEvt openyuv(EvtTYPE2);
	openyuv.setParam("CommandName", "open_yuvfile");
	openyuv.setParam("yuv_filePath", FliePath);
	openyuv.setParam("yuv_width", paramselect.getyuvwidth());
	openyuv.setParam("yuv_height", paramselect.getyuvheight());
	openyuv.setParam("yuv_format", paramselect.getformattype());
	openyuv.setParam("yuv_layer", QVariant::fromValue(0));//普通打开yuv，默认layer=0;
	openyuv.setParam("yuv_decoded", true);
	openyuv.dispatch();	
	//OpenNum = -1;
}

void longmanApp::on_actionAbout_triggered()
{
	QMessageBox::about(this, QStringLiteral("关于程序"), QStringLiteral("这是Longman的测试程序"));
}

void longmanApp::on_actionAboutQT_triggered()
{
	QMessageBox::aboutQt(this);
}

void longmanApp::on_nextButton_clicked()
{
	int curpoc = ui.FrameIdxSlider->value();
	++curpoc;
	ui.FrameIdxSlider->setValue(curpoc);
}

void longmanApp::on_backButton_clicked()
{
	int curpoc = ui.FrameIdxSlider->value();
	--curpoc;
	ui.FrameIdxSlider->setValue(curpoc);
}

void longmanApp::on_FrameIdxSlider_valueChanged(int poc)
{
	sendEvttoChnagePOC(poc);
}

void longmanApp::on_beginButton_clicked()
{
	ui.FrameIdxSlider->setValue(0);
}

void longmanApp::on_endButton_clicked()
{
	ui.FrameIdxSlider->setValue(ui.FrameIdxSlider->maximum()-1);
}

void longmanApp::player(int playerFrame)
{
	++playerFrame;
	ui.FrameIdxSlider->setValue(playerFrame);
	if (playerFrame>= ui.FrameIdxSlider->maximum())
		on_stopButton_clicked();
	
}

void longmanApp::on_playButton_clicked()
{
 	int currFrame = ui.FrameIdxSlider->value();
	int endFrame = ui.FrameIdxSlider->maximum();
	timeLine.setFrameRange(0, endFrame);
	double totalSec = (endFrame - currFrame) / (double)ui.FrameRateBox->value();
 	if (!totalSec|| timeLine.state()== QTimeLine::Running)
		{
			on_stopButton_clicked();
			return;
		}
	timeLine.setDuration(totalSec * 1000);
	timeLine.setStartFrame(currFrame);
	timeLine.setCurveShape(QTimeLine::LinearCurve);
	ui.playButton->setEnabled(true);
	ui.stopButton->setEnabled(true);
	ui.nextButton->setEnabled(false);
	ui.backButton->setEnabled(false);
	ui.beginButton->setEnabled(false);
	ui.endButton->setEnabled(false);
	timeLine.start();
}

void longmanApp::on_stopButton_clicked()
{
	timeLine.stop();
	ui.nextButton->setEnabled(true);
	ui.backButton->setEnabled(true);
	ui.beginButton->setEnabled(true);
	ui.endButton->setEnabled(true);
	ui.playButton->setEnabled(true);

}

void longmanApp::on_actionSave_as_image_triggered()
{
	QFileDialog dialog(this, QStringLiteral("保存为图片"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	if (openFailed) {
		const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
		dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	}
	QStringList mimeTypeFilters;
	const QByteArrayList supportedReadImage = QImageWriter::supportedMimeTypes();
	//所有读取支持的图片;
	mimeTypeFilters.push_back("image/bmp");//bmp图片; 
	mimeTypeFilters.push_back("image/jpeg");//jpeg图片;
	
	dialog.setMimeTypeFilters(mimeTypeFilters);
	dialog.setDefaultSuffix("bmp");
	QString FliePath; //QString ImageName;
	QByteArray tempByte;//转为CPP接口
	std::string PathCpp;//转为CPP接口
	if (dialog.exec() == QDialog::Accepted && !dialog.selectedFiles().isEmpty())
	{
		FliePath = dialog.selectedFiles().first();
		QImageWriter writer(FliePath);
		writer.write(*imageSave);
	}
}

void longmanApp::on_f1Button_clicked()
{
	static bool showdataEnable = false;
	//showdataEnable标志，当查看yuv的pixel值功能打开时，禁用部分功能;
	if (!showdataEnable)
		m_DataView->show();
	else
		m_DataView->hide();	
	ui.YUVgroupBox->setEnabled(showdataEnable);
	ui.actionOpen->setEnabled(showdataEnable);
	ui.f2Button->setEnabled(showdataEnable);
	ui.f4Button->setEnabled(showdataEnable);
	ui.actionOpen_SHVC_bitstream->setEnabled(showdataEnable);
	ui.groupBox->setEnabled(showdataEnable);
	longmanEvt showdata(EvtTYPE2);
	showdata.setParam("CommandName", "show_yuvdata");
	showdata.setParam("enabledByButton", !showdataEnable);
	showdata.dispatch();
	showdataEnable = !showdataEnable;
}

void longmanApp::on_f2Button_clicked()
{
	on_actionOpen_triggered();
}

void longmanApp::on_f3Button_clicked()
{
	on_actionSave_as_image_triggered();
}

void longmanApp::on_f4Button_clicked()
{
	on_actionOpen_SHVC_bitstream_triggered();
}

void longmanApp::on_f5Button_clicked()
{
	//显示并操作，已经打开成功的yuv;
	//这些yuv信息位于cmdProcessThread对象中;
	//计划的功能有:列出所有yuv,临时查看窗口,将信息保存为文件等等;
	qInfo()<< QStringLiteral("显示并操作已经打开成功的yuv;这些yuv信息位于cmdProcessThread对象中;计划的功能有:列出所有yuv,临时查看窗口,将信息保存为文件等;");

}
void longmanApp::on_f6Button_clicked()
{
	static bool showCUdepthEnable = false;
	longmanEvt showcuDepth(EvtTYPE2);
	showcuDepth.setParam("CommandName", "show_cuDepth");
	showcuDepth.setParam("enabledByButton", !showCUdepthEnable);
	showcuDepth.dispatch();
	showCUdepthEnable = !showCUdepthEnable;
}

