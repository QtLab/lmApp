#include "lmParseStreamPro.h"
//#include "..\\lib_cfg.h"
#include <QDir>

lmParseStreamPro::lmParseStreamPro(QObject *parent)
	: QObject(parent)
	//mCallDecoderEXE(this)
{
#if 0
	lmAllDecInfo *decinfo = lmAllDecInfo::getInstance();
	decinfo->setCachepath(sCache.toStdString());
#endif
}

lmParseStreamPro::~lmParseStreamPro()
{
	mCallDecoderEXE.kill();
}

bool lmParseStreamPro::decoderBitstream(const std::string & rstrBitPath, int layernum)
{
	QDir cCurDir = QDir::current();
	QString strdecoderpath = decoderPath;
	QString strdecoderpathabslout = QDir(strdecoderpath).absolutePath();
	QString bitstreamPath(QString::fromStdString(rstrBitPath));
	QString cachefolder = QDir(sCache).absolutePath();
	//QString cachefolder = QDir(QString("E://Longman//cache")).absolutePath();
	mCallDecoderEXE.setWorkingDirectory(cachefolder);// -o0 bl_yuv.yuv -o1 el_yuv.yuv
	

	QString arguments = QString("-ls %1 " ).arg(layernum + 1);
	for (size_t i = 0; i < layernum + 1; i++)
	{
		arguments += QString("-o%1 rec_layer%2.yuv ").arg(i).arg(i);
	}
	QString allCommand = QString("\"%1\" -b \"%2\" %3").arg(strdecoderpathabslout).arg(bitstreamPath).arg(arguments);
	mCallDecoderEXE.start(allCommand);
	mCallDecoderEXE.waitForFinished(-1);
	bool exeret = mCallDecoderEXE.exitCode()==0;
	return exeret;
}


bool lmParseStreamPro::preDec(const std::string & rstrBitPath)
{
	QString strdecoderpath = decoderPath;
	QString strdecoderpathabslout = QDir(strdecoderpath).absolutePath();
	QString bitstreamPath(QString::fromStdString(rstrBitPath));
	QString arguments = "-pdec 1";
	QString cachefolder = QDir(sCache).absolutePath();
	mCallDecoderEXE.setWorkingDirectory(cachefolder);// -o0 bl_yuv.yuv -o1 el_yuv.yuv
	QString allCommand = QString("\"%1\" -b \"%2\" %3").arg(strdecoderpathabslout).arg(bitstreamPath).arg(arguments);
	mCallDecoderEXE.start(allCommand);
	mCallDecoderEXE.waitForFinished(-1);
	bool exeret = mCallDecoderEXE.exitCode() == 0;
	return exeret;
}

void lmParseStreamPro::stopDecoding()
{
	mCallDecoderEXE.kill();
}

