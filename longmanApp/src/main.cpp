#include <QtWidgets/QApplication>
#include<QResource>
#include "lib_cfg.h"
#include "src\lmController.h"
#include "appGUI\longmanApp.h"
#include"appGUI\norwegianwoodstyle.h"
int main(int argc, char *argv[])
{
	//将debug信息输出句柄与某个函数连接;
	qInstallMessageHandler(longmanApp::xMessageOutput);
	QApplication a(argc, argv);
	//QApplication::setStyle(new NorwegianWoodStyle);
	QIcon icon;
	icon.addFile(QStringLiteral(":/appicon/app.ico"), QSize(), QIcon::Normal, QIcon::Off);
	QApplication::setWindowIcon(icon);
	lmController::getInstance();
	longmanApp w;
	w.show();

	return a.exec();
}
