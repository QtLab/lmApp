#ifndef lmParseStreamPro_h__
#define lmParseStreamPro_h__
#include <QObject>
#include <QProcess>
class lmParseStreamPro : public QObject
{
	Q_OBJECT

public:
	lmParseStreamPro(QObject *parent=Q_NULLPTR);
	~lmParseStreamPro();
	bool decoderBitstream(const std::string &,int);
private:
	QProcess mCallDecoderEXE;
};
#endif // lmParseStreamPro_h__
