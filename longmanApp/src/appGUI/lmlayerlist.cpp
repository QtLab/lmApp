#include "lmlayerlist.h"
//命令构造说明;
//1.("CommandName","addressLayer_list")+("maxLayer",int);
lmLayerList::lmLayerList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lmLayerList),
	lmView(nullptr)

{
    ui->setupUi(this);
	setModelName("List_View_Model");
	
	CallBackFunc pc = std::bind(&lmLayerList::getLayer, this, std::placeholders::_1);
	listenParam("addressLayer_list", pc);
}

lmLayerList::~lmLayerList()
{
    delete ui;
}

bool lmLayerList::getLayer(longmanEvt & pEvt)
{
	maxLayer = pEvt.getParam("maxLayer").toInt();
	for (size_t i = 0; i < maxLayer+1; i++)
	{
		
		ui->listWidget->addItem(new QListWidgetItem(QString("Layer%1").arg(i)));
	}
	return true;
}
