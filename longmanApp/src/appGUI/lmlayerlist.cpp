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
	QObject::connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(LayerClicked(QListWidgetItem *)));
}

lmLayerList::~lmLayerList()
{
    delete ui;
}

bool lmLayerList::getLayer(longmanEvt & pEvt)
{
	ui->listWidget->clear();
	allLayer.clear();
	maxLayer = pEvt.getParam("maxLayer").toInt();
	QListWidgetItem* tLayer = nullptr;
	for (size_t i = 0; i < maxLayer+1; i++)
	{
		tLayer = new QListWidgetItem(QString("Layer%1").arg(i));
		allLayer.push_back(tLayer);
		ui->listWidget->addItem(tLayer);
	}
	return true;
}

void lmLayerList::LayerClicked(QListWidgetItem *pLayer)
{
	QString listtxt = pLayer->text();
	int itBeSelect = -1;
	for (size_t i = 0; i < maxLayer + 1; i++)
	{
		if (listtxt == QString("Layer%1<-").arg(i))
			return;
		if (listtxt == QString("Layer%1").arg(i))
			{
				itBeSelect = i;
				pLayer->setText(listtxt + "<-");
			}
		else
		{
			allLayer[i]->setText(QString("Layer%1").arg(i));
		}
	}
	if (itBeSelect==-1)
	{
		int x = 0;
	}
	longmanEvt layer(EvtTYPE2);
	layer.setParam("CommandName", "parse_LayerToshow");
	layer.setParam("layerIdx", itBeSelect);
	layer.dispatch();
}
