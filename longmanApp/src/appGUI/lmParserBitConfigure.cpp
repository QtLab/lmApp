#include "lmParserBitConfigure.h"
#include "src/lmmodel.h"
#include <fstream>
static int openBitsreamnum = -1;
static QString lastbsf="str.bin";
lmParserBitConfigure::lmParserBitConfigure(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setModelName("BitStreamCfg_View_Model");
	CallBackFunc pcEvtHandle = std::bind(&lmParserBitConfigure::handleEvt, this, std::placeholders::_1);
	listenParam("Get_Layer", pcEvtHandle);
	ui.layerSelectArea->setEnabled(false);
}

lmParserBitConfigure::~lmParserBitConfigure()
{
}

bool lmParserBitConfigure::getcfg(QString & bitstreampath)
{
	ui.layerSelectArea->setEnabled(false);
	ui.bitstreamPath->setEnabled(true);
	QString bsf = ui.lineEdit->text();
	if (bsf.isEmpty())
	{
		openBitsreamnum = -1;
		return false;
	}
	QString suffixOffile;
	for (auto i = bsf.end() - 1; *i != QString("."); --i)
		suffixOffile.push_front(*i);
	if (suffixOffile != QString("bin"))
	{
		openBitsreamnum = 0;
		ui.lineEdit->setText(lastbsf);
		return false;
	}
	std::string cpppath = bsf.toStdString();
	std::ofstream tryRead(cpppath,std::ios::in);
	if (tryRead.fail())
	{
		longmanEvt openstbitstream(EvtTYPE1);
		openstbitstream.setParam("CommandName", "show_message");
		openstbitstream.setParam("MsgType", 2);
		openstbitstream.setParam("info", QStringLiteral("bitstream open failed!"));
		openstbitstream.dispatch();
		openBitsreamnum = 0;
		ui.lineEdit->setText(lastbsf);
		return false;
	}
	openBitsreamnum = 1;
	bitstreampath = bsf;
	lastbsf = bsf;
	return true;
}

bool lmParserBitConfigure::handleEvt(longmanEvt& rEvt)
{
	paramlist::const_iterator paramBeg;
	paramlist::const_iterator paramEnd;
	rEvt.getParamIter(paramBeg, paramEnd);
	bool hideFlag = false;
	for (auto i = paramBeg; i != paramEnd; ++i)
	{
		if (i->first == "MaxLayer")
			{
				mMaxLayer=i->second.toInt();
				ui.layerSelectArea->setEnabled(true);
				ui.bitstreamPath->setEnabled(false);
				if (exec() != Accepted)
					return false;
				decCammand();
				return true;
			}
	}

	return false;
}

void lmParserBitConfigure::decCammand()
{
	//���θ���Maxlayer��ѡ��;
	
	if (mMaxLayer>8)
	{
		int x = 0;
	}
}

void lmParserBitConfigure::on_toolButton_clicked()
{
	QFileDialog dialog(this, QStringLiteral("open bitstream"));
	if (openBitsreamnum == -1) 
	{
		const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
		dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	}
	dialog.setNameFilter(QStringLiteral("bitstream file (*.bin)"));
	if (dialog.exec()== QDialog::Accepted&& !dialog.selectedFiles().isEmpty())
		{
			QString bitstreamPath = dialog.selectedFiles().first();
			ui.lineEdit->setText(bitstreamPath);
		}
}