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
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_lmParserBitConfigure
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *bitstreamPath;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QToolButton *toolButton;
    QLabel *label_4;
    QGroupBox *layerSelectArea;
    QHBoxLayout *LayerArea;
    QLabel *label_2;
    QVBoxLayout *layer_0;
    QCheckBox *checkBox;
    QLabel *label_3;
    QVBoxLayout *layer_1;
    QCheckBox *checkBox_4;
    QLabel *label_7;
    QVBoxLayout *layer_2;
    QCheckBox *checkBox_5;
    QLabel *label_8;
    QVBoxLayout *layer_3;
    QCheckBox *checkBox_6;
    QLabel *label_9;
    QVBoxLayout *layer_4;
    QCheckBox *checkBox_7;
    QLabel *label_10;
    QVBoxLayout *layer_5;
    QCheckBox *checkBox_2;
    QLabel *label_5;
    QVBoxLayout *layer_6;
    QCheckBox *checkBox_8;
    QLabel *label_11;
    QVBoxLayout *layer_7;
    QCheckBox *checkBox_9;
    QLabel *label_12;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *lmParserBitConfigure)
    {
        if (lmParserBitConfigure->objectName().isEmpty())
            lmParserBitConfigure->setObjectName(QStringLiteral("lmParserBitConfigure"));
        lmParserBitConfigure->resize(400, 150);
        lmParserBitConfigure->setMinimumSize(QSize(400, 150));
        lmParserBitConfigure->setMaximumSize(QSize(400, 150));
        verticalLayout = new QVBoxLayout(lmParserBitConfigure);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        bitstreamPath = new QGroupBox(lmParserBitConfigure);
        bitstreamPath->setObjectName(QStringLiteral("bitstreamPath"));
        horizontalLayout = new QHBoxLayout(bitstreamPath);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(bitstreamPath);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(bitstreamPath);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        toolButton = new QToolButton(bitstreamPath);
        toolButton->setObjectName(QStringLiteral("toolButton"));

        horizontalLayout->addWidget(toolButton);

        label_4 = new QLabel(bitstreamPath);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout->addWidget(label_4);


        verticalLayout->addWidget(bitstreamPath);

        layerSelectArea = new QGroupBox(lmParserBitConfigure);
        layerSelectArea->setObjectName(QStringLiteral("layerSelectArea"));
        LayerArea = new QHBoxLayout(layerSelectArea);
		LayerArea->setSpacing(6);
		LayerArea->setContentsMargins(11, 11, 11, 11);
		LayerArea->setObjectName(QStringLiteral("LayerArea"));
        LayerArea->setContentsMargins(-1, -1, -1, 3);
        label_2 = new QLabel(layerSelectArea);
        label_2->setObjectName(QStringLiteral("label_2"));

        LayerArea->addWidget(label_2);

        layer_0 = new QVBoxLayout();
        layer_0->setSpacing(6);
        layer_0->setObjectName(QStringLiteral("layer_0"));
        checkBox = new QCheckBox(layerSelectArea);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        layer_0->addWidget(checkBox);

        label_3 = new QLabel(layerSelectArea);
        label_3->setObjectName(QStringLiteral("label_3"));

        layer_0->addWidget(label_3);


        LayerArea->addLayout(layer_0);

        layer_1 = new QVBoxLayout();
        layer_1->setSpacing(6);
        layer_1->setObjectName(QStringLiteral("layer_1"));
        checkBox_4 = new QCheckBox(layerSelectArea);
        checkBox_4->setObjectName(QStringLiteral("checkBox_4"));

        layer_1->addWidget(checkBox_4);

        label_7 = new QLabel(layerSelectArea);
        label_7->setObjectName(QStringLiteral("label_7"));

        layer_1->addWidget(label_7);


        LayerArea->addLayout(layer_1);

        layer_2 = new QVBoxLayout();
        layer_2->setSpacing(6);
        layer_2->setObjectName(QStringLiteral("layer_2"));
        checkBox_5 = new QCheckBox(layerSelectArea);
        checkBox_5->setObjectName(QStringLiteral("checkBox_5"));

        layer_2->addWidget(checkBox_5);

        label_8 = new QLabel(layerSelectArea);
        label_8->setObjectName(QStringLiteral("label_8"));

        layer_2->addWidget(label_8);


        LayerArea->addLayout(layer_2);

        layer_3 = new QVBoxLayout();
        layer_3->setSpacing(6);
        layer_3->setObjectName(QStringLiteral("layer_3"));
        checkBox_6 = new QCheckBox(layerSelectArea);
        checkBox_6->setObjectName(QStringLiteral("checkBox_6"));

        layer_3->addWidget(checkBox_6);

        label_9 = new QLabel(layerSelectArea);
        label_9->setObjectName(QStringLiteral("label_9"));

        layer_3->addWidget(label_9);


        LayerArea->addLayout(layer_3);

        layer_4 = new QVBoxLayout();
        layer_4->setSpacing(6);
        layer_4->setObjectName(QStringLiteral("layer_4"));
        checkBox_7 = new QCheckBox(layerSelectArea);
        checkBox_7->setObjectName(QStringLiteral("checkBox_7"));

        layer_4->addWidget(checkBox_7);

        label_10 = new QLabel(layerSelectArea);
        label_10->setObjectName(QStringLiteral("label_10"));

        layer_4->addWidget(label_10);


        LayerArea->addLayout(layer_4);

        layer_5 = new QVBoxLayout();
        layer_5->setSpacing(6);
        layer_5->setObjectName(QStringLiteral("layer_5"));
        checkBox_2 = new QCheckBox(layerSelectArea);
        checkBox_2->setObjectName(QStringLiteral("checkBox_2"));

        layer_5->addWidget(checkBox_2);

        label_5 = new QLabel(layerSelectArea);
        label_5->setObjectName(QStringLiteral("label_5"));

        layer_5->addWidget(label_5);


        LayerArea->addLayout(layer_5);

        layer_6 = new QVBoxLayout();
        layer_6->setSpacing(6);
        layer_6->setObjectName(QStringLiteral("layer_6"));
        checkBox_8 = new QCheckBox(layerSelectArea);
        checkBox_8->setObjectName(QStringLiteral("checkBox_8"));

        layer_6->addWidget(checkBox_8);

        label_11 = new QLabel(layerSelectArea);
        label_11->setObjectName(QStringLiteral("label_11"));

        layer_6->addWidget(label_11);


        LayerArea->addLayout(layer_6);

        layer_7 = new QVBoxLayout();
        layer_7->setSpacing(6);
        layer_7->setObjectName(QStringLiteral("layer_7"));
        checkBox_9 = new QCheckBox(layerSelectArea);
        checkBox_9->setObjectName(QStringLiteral("checkBox_9"));

        layer_7->addWidget(checkBox_9);

        label_12 = new QLabel(layerSelectArea);
        label_12->setObjectName(QStringLiteral("label_12"));

        layer_7->addWidget(label_12);


        LayerArea->addLayout(layer_7);


        verticalLayout->addWidget(layerSelectArea);

        buttonBox = new QDialogButtonBox(lmParserBitConfigure);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(lmParserBitConfigure);
        QObject::connect(buttonBox, SIGNAL(accepted()), lmParserBitConfigure, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), lmParserBitConfigure, SLOT(reject()));

        QMetaObject::connectSlotsByName(lmParserBitConfigure);
    } // setupUi

    void retranslateUi(QDialog *lmParserBitConfigure)
    {
        lmParserBitConfigure->setWindowTitle(QApplication::translate("lmParserBitConfigure", "lmParserBitConfigure", 0));
        label->setText(QApplication::translate("lmParserBitConfigure", "bitstreampath", 0));
        toolButton->setText(QApplication::translate("lmParserBitConfigure", "...", 0));
        label_4->setText(QApplication::translate("lmParserBitConfigure", "-b x", 0));
        label_2->setText(QApplication::translate("lmParserBitConfigure", "LayertoDec", 0));
        checkBox->setText(QString());
        label_3->setText(QApplication::translate("lmParserBitConfigure", "0", 0));
        checkBox_4->setText(QString());
        label_7->setText(QApplication::translate("lmParserBitConfigure", "1", 0));
        checkBox_5->setText(QString());
        label_8->setText(QApplication::translate("lmParserBitConfigure", "2", 0));
        checkBox_6->setText(QString());
        label_9->setText(QApplication::translate("lmParserBitConfigure", "3", 0));
        checkBox_7->setText(QString());
        label_10->setText(QApplication::translate("lmParserBitConfigure", "4", 0));
        checkBox_2->setText(QString());
        label_5->setText(QApplication::translate("lmParserBitConfigure", "5", 0));
        checkBox_8->setText(QString());
        label_11->setText(QApplication::translate("lmParserBitConfigure", "6", 0));
        checkBox_9->setText(QString());
        label_12->setText(QApplication::translate("lmParserBitConfigure", "7", 0));
    } // retranslateUi

};

namespace Ui {
    class lmParserBitConfigure: public Ui_lmParserBitConfigure {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMPARSERBITCONFIGURE_H
