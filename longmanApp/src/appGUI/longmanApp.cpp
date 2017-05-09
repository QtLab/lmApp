#include "longmanApp.h"
#include<QImageWriter>
//�����˵��;
//1.("CommandName","update_mainwindow")+("yuv_currentPOC",int)+("yuv_totalFrames",int)+;
//("yuv_width",int)+("yuv_height",int)+("image",uchar*)+;
//һ��ֻ�ڳɹ���yuv�ļ��ɹ���ִ��һ��,���ڸ�����������ʾ��yuv��Ϣ,�Լ���ǰ��ʾ��ͼƬ��ָ��;
//���ڱ���ΪͼƬ;
const char *formattext[] = { "400","420","444" };
static int OpenNum = -1;
static int formattype = 1;
longmanApp::longmanApp(QWidget *parent)
	: QMainWindow(parent),
	lmView(nullptr),
	m_imageView(new lmGraphView(this)),
	msgBox(new lmMessageBox(this)),
	imageSave(nullptr),
	m_DataView(new lmDataView)
{
	timeLine.stop();
	ui.setupUi(this);
	//QIcon icon;
	//icon.addFile(QStringLiteral(":/appicon/app.ico"), QSize(), QIcon::Normal, QIcon::Off);
	//setWindowIcon(icon);
	//m_DataView->setWindowIcon(icon);
	
	setCentralWidget(m_imageView);
	CallBackFunc pcupdate = std::bind(&longmanApp::updatemainwindow, this, std::placeholders::_1);
	listenParam("update_mainwindow", pcupdate);
	setModelName("MainWindow_View_Model");
	//
	//connect(ui.FrameIdxSlider, SIGNAL(ValueChanged(int)), this, SLOT(on_FrameIdxSlider_ValueChanged(int)));
	//
	ui.YUVgroupBox->setEnabled(false);
	ui.stopButton->setEnabled(false);
	ui.actionSave_as_image->setEnabled(false);
	ui.f1Button->setEnabled(false);
	ui.dockWidget->setWidget(m_DataView);
	//
	
	connect(&timeLine, SIGNAL(frameChanged(int)), this, SLOT(player(int)));
}

bool longmanApp::updatemainwindow(longmanEvt& updateWinEvt)
{
	++OpenNum;
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
	ui.FrameIdxSlider->setValue(curpoc + 1);//������������;
	ui.f1Button->setEnabled(true);
	//
	
	return true;
}

void longmanApp::on_actionOpen_triggered()
{
	if (OpenNum==-1)
	{
		ui.FrameIdxSlider->setMinimum(-1);
		ui.FrameIdxSlider->setValue(-1);
	}
	QFileDialog dialog(this, QStringLiteral("��YUV�ļ�"));
	if (OpenNum == -1) {
		const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
		dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	}
	dialog.setNameFilter(QStringLiteral("YUV �ļ� (*.yuv)"));//�ļ���ʽ����;
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
		longmanEvt paramWarning(EvtTYPE1);
		paramWarning.setParam("CommandName", "show_message");
		paramWarning.setParam("MsgType", 2);
		paramWarning.setParam("info", QStringLiteral("�ߴ粻��Ϊ������"));
		paramWarning.dispatch();
		return;
	}
	longmanEvt openyuv(EvtTYPE2);
	openyuv.setParam("CommandName", "open_yuvfile");
	openyuv.setParam("yuv_filePath", FliePath);
	openyuv.setParam("yuv_width", paramselect.getyuvwidth());
	openyuv.setParam("yuv_height", paramselect.getyuvheight());
	openyuv.setParam("yuv_format", paramselect.getformattype());
	openyuv.dispatch();	
	OpenNum = -1;
}

void longmanApp::on_actionAbout_triggered()
{
	QMessageBox::about(this, QStringLiteral("���ڳ���"), QStringLiteral("����Longman�Ĳ��Գ���"));
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
	int curpoc = poc;
	longmanEvt yuvnext(EvtTYPE2);
	yuvnext.setParam("CommandName", "change_imagepoc");
	yuvnext.setParam("yuv_POC", curpoc);
	if (OpenNum == -1)//��ʧ�ܣ�����;
		{
			yuvnext.setParam("force_readData", true);
			yuvnext.setParam("recoverLast", true);
			OpenNum = 1;
		}
	else if (OpenNum==1)//�򿪳ɹ�����ˢ�µ�һ֡;
		{
			yuvnext.setParam("force_readData", true);
			yuvnext.setParam("recoverLast", false);
		}
	else if (OpenNum==0)//�򿪳ɹ���δˢ�µ�һ֡;
		{
			yuvnext.setParam("force_readData", true);
			yuvnext.setParam("recoverLast", false);
			OpenNum = 1;
			ui.FrameIdxSlider->setMinimum(0);
		}
	yuvnext.dispatch();
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
	QFileDialog dialog(this, QStringLiteral("����ΪͼƬ"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	if (!(OpenNum > 0)) {
		const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
		dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	}
	QStringList mimeTypeFilters;
	const QByteArrayList supportedReadImage = QImageWriter::supportedMimeTypes();
	//���ж�ȡ֧�ֵ�ͼƬ;
	mimeTypeFilters.push_back("image/bmp");//bmpͼƬ; 
	mimeTypeFilters.push_back("image/jpeg");//jpegͼƬ;
	
	dialog.setMimeTypeFilters(mimeTypeFilters);
	dialog.setDefaultSuffix("bmp");
	QString FliePath; //QString ImageName;
	QByteArray tempByte;//תΪCPP�ӿ�
	std::string PathCpp;//תΪCPP�ӿ�
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

	if (!showdataEnable)
	{
		m_DataView->show();
	}
	else
	{
		m_DataView->hide();	
	}
	longmanEvt showdata(EvtTYPE2);
	showdata.setParam("CommandName", "show_yuvdata");
	showdata.setParam("enabledByButton", true);
	showdata.dispatch();
	showdataEnable = !showdataEnable;
}
