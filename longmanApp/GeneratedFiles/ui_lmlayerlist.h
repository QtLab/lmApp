/********************************************************************************
** Form generated from reading UI file 'lmlayerlist.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LMLAYERLIST_H
#define UI_LMLAYERLIST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_lmLayerList
{
public:
    QGridLayout *gridLayout;
    QListWidget *listWidget;

    void setupUi(QWidget *lmLayerList)
    {
        if (lmLayerList->objectName().isEmpty())
            lmLayerList->setObjectName(QStringLiteral("lmLayerList"));
        lmLayerList->resize(175, 107);
        gridLayout = new QGridLayout(lmLayerList);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        listWidget = new QListWidget(lmLayerList);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        listWidget->setMinimumSize(QSize(96, 32));
        listWidget->setMaximumSize(QSize(150, 300));

        gridLayout->addWidget(listWidget, 0, 0, 1, 1);


        retranslateUi(lmLayerList);

        QMetaObject::connectSlotsByName(lmLayerList);
    } // setupUi

    void retranslateUi(QWidget *lmLayerList)
    {
        lmLayerList->setWindowTitle(QApplication::translate("lmLayerList", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class lmLayerList: public Ui_lmLayerList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMLAYERLIST_H
