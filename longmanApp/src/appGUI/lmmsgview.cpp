#include "lmmsgview.h"
#include "ui_lmmsgview.h"
const QMessageBox::Icon msgType[] = { QMessageBox::NoIcon,
QMessageBox::Warning,QMessageBox::Critical,QMessageBox::Critical,QMessageBox::Information,QMessageBox::Information };
lmMsgView::lmMsgView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lmMsgView)
{
    ui->setupUi(this);
	mWarningbox.setWindowTitle("Attention");
	setWindowTitle(tr("lmMsgView"));
	setModelName("Message_View_Model_inMW");
	CallBackFunc pcMsgEvtHandle = std::bind(&lmMsgView::handleMsg, this, std::placeholders::_1);
	listenParam("outputMsg", pcMsgEvtHandle);
}

lmMsgView::~lmMsgView()
{
    delete ui;
}

bool lmMsgView::handleMsg(longmanEvt& rEvt)
{
	bool msgInfo_hide = rEvt.getParam("isHide").toBool();
	if (msgInfo_hide)
	{
		//ʵ�ֵ����ڵ��Զ�����;
		mWarningbox.hide();
		ui->textBrowser->append("hide MsgBox");
		return true;
	}

	QtMsgType msgT = QtMsgType( rEvt.getParam("MsgType").toInt());
	QString msgInfo = rEvt.getParam("info").toString();
	if (msgT == QtDebugMsg|| msgT == QtInfoMsg)
		ui->textBrowser->setTextColor(QColor(Qt::black));
	else
		ui->textBrowser->setTextColor(QColor(Qt::red));
	if (msgT>= QtWarningMsg)
	{
		//�ǵ�����Ϣ������;
		mWarningbox.setIcon(msgType[msgT]);
		mWarningbox.setText(msgInfo);
		mWarningbox.show();
	}
	if (ui->textBrowser->backwardHistoryCount() > 5)
		ui->textBrowser->clear();
	ui->textBrowser->append(msgInfo);

	return true;
}
