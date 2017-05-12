/********************************************************************************
** Form generated from reading UI file 'lmParserBitConfigure.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LMPARSERBITCONFIGURE_H
#define UI_LMPARSERBITCONFIGURE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_lmParserBitConfigure
{
public:

    void setupUi(QDialog *lmParserBitConfigure)
    {
        if (lmParserBitConfigure->objectName().isEmpty())
            lmParserBitConfigure->setObjectName(QStringLiteral("lmParserBitConfigure"));
        lmParserBitConfigure->resize(400, 300);

        retranslateUi(lmParserBitConfigure);

        QMetaObject::connectSlotsByName(lmParserBitConfigure);
    } // setupUi

    void retranslateUi(QDialog *lmParserBitConfigure)
    {
        lmParserBitConfigure->setWindowTitle(QApplication::translate("lmParserBitConfigure", "lmParserBitConfigure", 0));
    } // retranslateUi

};

namespace Ui {
    class lmParserBitConfigure: public Ui_lmParserBitConfigure {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMPARSERBITCONFIGURE_H
