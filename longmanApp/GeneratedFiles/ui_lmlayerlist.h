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
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_lmLayerList
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QListWidget *listWidget;

    void setupUi(QWidget *lmLayerList)
    {
        if (lmLayerList->objectName().isEmpty())
            lmLayerList->setObjectName(QStringLiteral("lmLayerList"));
        lmLayerList->resize(155, 210);
        verticalLayout = new QVBoxLayout(lmLayerList);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(lmLayerList);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);

        listWidget = new QListWidget(lmLayerList);
        listWidget->setObjectName(QStringLiteral("listWidget"));

        verticalLayout->addWidget(listWidget);


        retranslateUi(lmLayerList);

        QMetaObject::connectSlotsByName(lmLayerList);
    } // setupUi

    void retranslateUi(QWidget *lmLayerList)
    {
        lmLayerList->setWindowTitle(QApplication::translate("lmLayerList", "Form", 0));
        label->setText(QApplication::translate("lmLayerList", "\345\261\202\347\272\247\345\210\227\350\241\250", 0));
    } // retranslateUi

};

namespace Ui {
    class lmLayerList: public Ui_lmLayerList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMLAYERLIST_H
