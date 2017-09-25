#include "lmlayerlist.h"
//命令构造说明;
//1.("CommandName","addressLayer_list")+("maxLayer",int);
const static std::string prefixLayer = "Layer";
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
	clearAllListitem();
	maxLayer = pEvt.getParam("maxLayer").toInt();
	int layerDec = pEvt.getParam("decLayer").toInt();
	//获得需要显示的层级;
	layerDec &= 0x0f;
	std::vector<int> layerIdxToDec;
	int t = 0;
	for (size_t i = 0; i < 8; i++)
	{
		t = (layerDec >> i) & 0x01;
		if (t == 1)
			layerIdxToDec.push_back(i);
	}
	QListWidgetItem* tLayer = nullptr;
	QString itemName;
	for (size_t i = 0; i < layerIdxToDec.size(); i++)
	{
		itemName = QString("%1_%2").arg(QString::fromStdString(prefixLayer)).arg(layerIdxToDec[i]);
		tLayer = new QListWidgetItem(itemName);
		ui->listWidget->addItem(tLayer);
		allLayer.insert({ tLayer,layerIdxToDec[i] });
	}
	return true;
}

void lmLayerList::clearAllListitem()
{
	if (!allLayer.empty())
	{
		for (auto i = allLayer.begin(); i != allLayer.end(); ++i)
			delete i->first;
		allLayer.clear();
	}
}

// bool lmLayerList::addOpenedYUV(longmanEvt & pEvt)
// {
// 	return true;
// }

void lmLayerList::LayerClicked(QListWidgetItem *pLayer)
{
	QString listtxt = pLayer->text();
	QString temStr;
	auto pLayerit = allLayer.find(pLayer);
	if (pLayerit==allLayer.end())
		return;
	for (auto i = allLayer.begin(); i != allLayer.end(); ++i)
	{
		temStr = QString("%1_%2").arg(QString::fromStdString(prefixLayer)).arg(i->second);
		if (i==pLayerit)
		{
			if (listtxt == temStr + "<-")
			{
				return;
			}
			else
			{
				pLayer->setText(temStr+ "<-");
			}
			continue;
		}
		i->first->setText(temStr);
	}
	longmanEvt layer(EvtTYPE2);
	layer.setParam("CommandName", "parse_LayerToshow");
	layer.setParam("layerIdx", pLayerit->second);
	layer.dispatch();
}
