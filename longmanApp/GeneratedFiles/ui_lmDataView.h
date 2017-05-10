/********************************************************************************
** Form generated from reading UI file 'lmDataView.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LMDATAVIEW_H
#define UI_LMDATAVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_lmDataView
{
public:

    void setupUi(QWidget *lmDataView)
    {
        if (lmDataView->objectName().isEmpty())
            lmDataView->setObjectName(QStringLiteral("lmDataView"));
        lmDataView->resize(400, 300);

        retranslateUi(lmDataView);

        QMetaObject::connectSlotsByName(lmDataView);
    } // setupUi

    void retranslateUi(QWidget *lmDataView)
    {
        lmDataView->setWindowTitle(QApplication::translate("lmDataView", "lmDataView", 0));
    } // retranslateUi

};

namespace Ui {
    class lmDataView: public Ui_lmDataView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMDATAVIEW_H
