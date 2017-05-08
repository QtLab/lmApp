#pragma once

#ifndef lmMessageBox_h__
#define lmMessageBox_h__

#include <QMessageBox>
#include "src/lmView.h"
//���ã���ʾϵͳ��Ϣ;
//��Ӧ����:EvtTYPE1�����¼�,��һ�����Ϊ("CommandName","show_message");
//��������:("MsgType",0(��1��2)),0����ͨ��1:���棬2������;
//("info","text"),����;(mu)�����������Դ���
class lmMessageBox : public QMessageBox,public lmView
{
	Q_OBJECT

public:
	lmMessageBox(QWidget *parent);
	~lmMessageBox();
private:
	bool handleMsgEvt(longmanEvt&);
};
#endif // lmMessageBox_h__
