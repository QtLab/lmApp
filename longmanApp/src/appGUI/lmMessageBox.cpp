#include "lmMessageBox.h"
#include <QIcon>
//
const QMessageBox::Icon msgType[] = { QMessageBox::Information,
QMessageBox::Warning,QMessageBox::Critical };
lmMessageBox::lmMessageBox(QWidget *parent):
lmView(nullptr)
{
	//QIcon icon;
	//icon.addFile(QStringLiteral(":/appicon/app.ico"), QSize(), QIcon::Normal, QIcon::Off);
	//setWindowIcon(icon);
	setWindowTitle(tr("lmMsg"));
	setModelName("Message_View_Model");
	CallBackFunc pcMsgEvtHandle = std::bind(&lmMessageBox::handleMsgEvt, this, std::placeholders::_1);
	listenParam("show_message", pcMsgEvtHandle);
	setStandardButtons(QMessageBox::Ignore| QMessageBox::Close);
// 	setMinimumSize(this->size().width() / 4, this->size().height() / 4);
// 	setMinimumSize(this->size().width() / 2, this->size().height() / 2);
}

lmMessageBox::~lmMessageBox()
{
}

bool lmMessageBox::handleMsgEvt(longmanEvt& rMsgEvt)
{
	paramlist::const_iterator paramBeg;
	paramlist::const_iterator paramEnd;
	rMsgEvt.getParamIter(paramBeg, paramEnd);
	bool hideFlag = false;
	for (auto i = paramBeg; i != paramEnd; ++i)
	{
		if (i->first== "MsgType")
			setIcon(msgType[i->second.toInt()]);
		if (i->first == "info")
			setText(i->second.toString());
		if (i->first == "isHide")
			hideFlag = i->second.toBool();
	}
	if (!hideFlag)
		show();
	else
		hide();
	return true;
}
