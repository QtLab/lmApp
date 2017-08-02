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

void lmParserBitConfigure::resetState(bool bitpath/*=true*/)
{
	if (bitpath)
		//恢复为路径选择;
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
		//findChild函数可以根据名字返回类对象;
		QString chName = QString("checkBox_%1").arg(i);
		checkd = findChild<QCheckBox *>(QString(chName));
		if (checkd->isChecked())
		{
			layerIdxFlag.push_back(i);
		}
	}
	if (layerIdxFlag.empty())
		return false;
	//采用8bit传输解码层级信息,每bit对应层索引的标志;
	int layerflag = 0;
	for (int i = 0; i < layerIdxFlag.size(); i++)
	{
		layerflag += 1 << layerIdxFlag[i];
		layerflag = layerflag & 0x0f;
	}
	//发送编码信息;
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
	//恢复为层级选择;
	ui.layerSelectArea->setEnabled(true);
	ui.bitstreamPath->setEnabled(false);
	//处理复选框;
	//屏蔽高于Maxlayer的选项;
	QCheckBox * checkd = nullptr;
	for (size_t i = 0; i < 8; i++)
	{
		//findChild函数可以根据名字返回类对象;
		QString chName = QString("checkBox_%1").arg(i);
		checkd = findChild<QCheckBox *>(QString(chName));
		if (checkd!=nullptr&&i < mMaxLayer-1)
			//最高层级必须解码;
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