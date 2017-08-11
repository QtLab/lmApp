#include "lmParserBitConfigure.h"
#include "src/lmmodel.h"
#include <fstream>
static int openBitsreamnum = -1;
static QString lastbsf="str.bin";
lmParserBitConfigure::lmParserBitConfigure(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(tr("Bitstream"));
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowCloseButtonHint;
	setWindowFlags(flags);
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
		qWarning() << QStringLiteral("bitstream open failed!");
		openBitsreamnum = 0;
		ui.lineEdit->setText(lastbsf);
		return false;
	}
	openBitsreamnum = 1;
	bitstreampath = bsf;
	lastbsf = bsf;
	return true;
}

void lmParserBitConfigure::resetState(bool bitpath/*=true*/)
{
	if (bitpath)
		//�ָ�Ϊ·��ѡ��;
	{
		ui.layerSelectArea->setEnabled(false);
		ui.bitstreamPath->setEnabled(true);
	}
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
				proCheckBox();
				if (exec() == Accepted&&getLayertoDec())
					return true;
				
				return false;
			}
	}

	return false;
}

bool lmParserBitConfigure::getLayertoDec()
{
	QCheckBox * checkd = nullptr;
	std::vector<int> layerIdxFlag;
	for (int i = 0; i < mMaxLayer; i++)
	{
		//findChild�������Ը������ַ��������;
		QString chName = QString("checkBox_%1").arg(i);
		checkd = findChild<QCheckBox *>(QString(chName));
		if (checkd->isChecked())
		{
			layerIdxFlag.push_back(i);
		}
	}
	if (layerIdxFlag.empty())
		return false;
	//����8bit�������㼶��Ϣ,ÿbit��Ӧ�������ı�־;
	int layerflag = 0;
	for (int i = 0; i < layerIdxFlag.size(); i++)
	{
		layerflag += 1 << layerIdxFlag[i];
		layerflag = layerflag & 0x0f;
	}
	//���ͱ�����Ϣ;
	longmanEvt decBitstream(EvtTYPE2);
	decBitstream.setParam("CommandName", "parse_shvcbitstream");
	decBitstream.setParam("bitstream_path", lastbsf);
	decBitstream.setParam("numLayerToDec", layerflag);
	decBitstream.setParam("maxLayerIdx", mMaxLayer-1);
	decBitstream.dispatch();
	return true;
}

void lmParserBitConfigure::proCheckBox()
{
	//�ָ�Ϊ�㼶ѡ��;
	ui.layerSelectArea->setEnabled(true);
	ui.bitstreamPath->setEnabled(false);
	//����ѡ��;
	//���θ���Maxlayer��ѡ��;
	QCheckBox * checkd = nullptr;
	for (size_t i = 0; i < 8; i++)
	{
		//findChild�������Ը������ַ��������;
		QString chName = QString("checkBox_%1").arg(i);
		checkd = findChild<QCheckBox *>(QString(chName));
		if (checkd!=nullptr&&i < mMaxLayer-1)
			//��߲㼶�������;
			checkd->setEnabled(true);
		else
			checkd->setEnabled(false);
		if (i == mMaxLayer-1)
			checkd->setChecked(true);
	}

}

void lmParserBitConfigure::on_toolButton_clicked()
{
	QFileDialog dialog(this, QStringLiteral("open bitstream"));
	if (openBitsreamnum == -1) 
	{
		const QString defaultLocations = QDir::currentPath() + "/cache";
		dialog.setDirectory(defaultLocations.isEmpty() ? QDir::currentPath() : defaultLocations);
	}
	dialog.setNameFilter(QStringLiteral("bitstream file (*.bin)"));
	if (dialog.exec()== QDialog::Accepted&& !dialog.selectedFiles().isEmpty())
		{
			QString bitstreamPath = dialog.selectedFiles().first();
			ui.lineEdit->setText(bitstreamPath);
		}
}