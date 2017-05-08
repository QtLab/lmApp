#pragma once

#ifndef lmMessageBox_h__
#define lmMessageBox_h__

#include <QMessageBox>
#include "src/lmView.h"
//作用：显示系统消息;
//响应条件:EvtTYPE1类型事件,第一组参数为("CommandName","show_message");
//其他参数:("MsgType",0(或1或2)),0：普通，1:警告，2：错误;
//("info","text"),内容;(mu)后续参数忽略处理
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
