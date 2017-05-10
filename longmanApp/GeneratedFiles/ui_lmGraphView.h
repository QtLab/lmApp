/********************************************************************************
** Form generated from reading UI file 'lmGraphView.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LMGRAPHVIEW_H
#define UI_LMGRAPHVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_lmGraphView
{
public:

    void setupUi(QGraphicsView *lmGraphView)
    {
        if (lmGraphView->objectName().isEmpty())
            lmGraphView->setObjectName(QStringLiteral("lmGraphView"));
        lmGraphView->resize(400, 300);

        retranslateUi(lmGraphView);

        QMetaObject::connectSlotsByName(lmGraphView);
    } // setupUi

    void retranslateUi(QGraphicsView *lmGraphView)
    {
        lmGraphView->setWindowTitle(QApplication::translate("lmGraphView", "lmGraphView", 0));
    } // retranslateUi

};

namespace Ui {
    class lmGraphView: public Ui_lmGraphView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMGRAPHVIEW_H
