/********************************************************************************
** Form generated from reading UI file 'frmmain.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRMMAIN_H
#define UI_FRMMAIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_frmmain
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QLabel *lblOriginal;
    QPushButton *btnStart;

    void setupUi(QMainWindow *frmmain)
    {
        if (frmmain->objectName().isEmpty())
            frmmain->setObjectName(QString::fromUtf8("frmmain"));
        frmmain->resize(400, 300);
        centralWidget = new QWidget(frmmain);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lblOriginal = new QLabel(centralWidget);
        lblOriginal->setObjectName(QString::fromUtf8("lblOriginal"));
        lblOriginal->setAutoFillBackground(true);
        lblOriginal->setFrameShape(QFrame::Box);
        lblOriginal->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lblOriginal, 0, 0, 1, 1);

        btnStart = new QPushButton(centralWidget);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));

        gridLayout->addWidget(btnStart, 1, 0, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);

        frmmain->setCentralWidget(centralWidget);

        retranslateUi(frmmain);

        QMetaObject::connectSlotsByName(frmmain);
    } // setupUi

    void retranslateUi(QMainWindow *frmmain)
    {
        frmmain->setWindowTitle(QCoreApplication::translate("frmmain", "frmmain", nullptr));
        lblOriginal->setText(QString());
        btnStart->setText(QCoreApplication::translate("frmmain", "Start", nullptr));
    } // retranslateUi

};

namespace Ui {
    class frmmain: public Ui_frmmain {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRMMAIN_H
