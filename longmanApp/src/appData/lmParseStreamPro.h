#ifndef lmParseStreamPro_h__
#define lmParseStreamPro_h__
#include <QObject>
#include <QProcess>
#include "../../../SHM12.1/source/Lib/lmdecInfo/lmAllDecInfo.h"
class lmParseStreamPro : public QObject
{
	Q_OBJECT

public:
	lmParseStreamPro(QObject *parent=Q_NULLPTR);
	~lmParseStreamPro();
	bool decoderBitstream(const std::string &,int);
	bool preDec(const std::string &);
	void stopDecoding();
	static std::string getDecYUVName(int lsyerIdx);
	static std::string getDecYUVName();
private:
	QProcess mCallDecoderEXE;
	QString decoderPath = "..//SHM12.1//bin//vc2010//Win32//Debug//TAppDecoderAnalyser.exe";
	QString sCache = "..//cache//";
};
#endif // lmParseStreamPro_h__
