#include "lmParseStreamPro.h"
#include <QDir>
lmParseStreamPro::lmParseStreamPro(QObject *parent)
	: QObject(parent)//,
	//mCallDecoderEXE()
{

}

lmParseStreamPro::~lmParseStreamPro()
{
}

bool lmParseStreamPro::decoderBitstream(const std::string & rstrBitPath, int layernum)
{
	QDir cCurDir = QDir::current();
	QString strdecoderpath = "..\\SHM12.1\\bin\\vc2010\\Win32\\Debug\\TAppDecoderAnalyser.exe";
	QString strdecoderpathabslout = QDir(strdecoderpath).absolutePath();
	//QString cachefolder = QDir(QString("..\\cache")).absolutePath();
	QString cachefolder = QDir(QString("E:\\Longman\\cache")).absolutePath();
	mCallDecoderEXE.setWorkingDirectory(cachefolder);// -o0 bl_yuv.yuv -o1 el_yuv.yuv
	QString strDecoderCmd = QString("\"%1\" -b \"%2\" -ls 2").arg(strdecoderpathabslout).arg(QString::fromStdString(rstrBitPath));
	mCallDecoderEXE.start(strDecoderCmd);
	mCallDecoderEXE.waitForFinished(-1);
	return true;
}
