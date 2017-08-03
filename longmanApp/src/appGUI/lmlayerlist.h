#ifndef LMLAYERLIST_H
#define LMLAYERLIST_H
#include <QWidget>
#include <QLabel>
#include "src/lmView.h"
#include "ui_lmlayerlist.h"
namespace Ui {
class lmLayerList;
}

class lmLayerList : public QWidget, public lmView
{
    Q_OBJECT

public:
    explicit lmLayerList(QWidget *parent = 0);
    ~lmLayerList();
	bool getLayer(longmanEvt & upimage);
private:
    Ui::lmLayerList *ui;
	int maxLayer = 0;
	std::vector<QListWidgetItem *> allLayer;
private slots :
	void LayerClicked(QListWidgetItem *);
	
};

#endif // LMLAYERLIST_H
