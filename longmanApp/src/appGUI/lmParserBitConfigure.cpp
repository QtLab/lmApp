#include "lmParserBitConfigure.h"
#include "src/lmmodel.h"
#include <fstream>
static int openBitsreamnum = -1;
static QString lastbsf;
lmParserBitConfigure::lmParserBitConfigure(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

lmParserBitConfigure::~lmParserBitConfigure()
{
}

bool lmParserBitConfigure::getcfg(QString & bitstreampath, int & numlayer)
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
	numlayer = ui.spinBox->value();
	return true;
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