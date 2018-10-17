/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "renderwidget.hpp"

QT_BEGIN_NAMESPACE

class Ui_winMain
{
public:
    QAction *actionUser_Preferences;
    QAction *actionOpen;
    QAction *actionSave;
    QWidget *wCentral;
    QGridLayout *gridLayout;
    QWidget *wRenderOpts;
    QVBoxLayout *verticalLayout_7;
    QTabWidget *twRenderOpts;
    QWidget *tabRenderOptsWorld;
    QVBoxLayout *verticalLayout_3;
    QScrollArea *scarOpts;
    QWidget *scarwWorldOpts;
    QVBoxLayout *verticalLayout_2;
    QLabel *lbRS;
    QLineEdit *ledRS;
    QLabel *lbBgTex;
    QRadioButton *rbBgFromPreset;
    QLabel *lbBgPreset1;
    QRadioButton *rbBgFromFile;
    QWidget *wBgFromFile;
    QHBoxLayout *horizontalLayout;
    QLineEdit *ledBgPath;
    QPushButton *pbChooseBgPath;
    QSpacerItem *spWorldOpts;
    QWidget *tabRenderOptsObject;
    QVBoxLayout *verticalLayout;
    QStackedWidget *stwObjectOpts;
    QWidget *pageRn;
    QVBoxLayout *verticalLayout_5;
    QScrollArea *scarRingOpts;
    QWidget *scarwRingOpts;
    QVBoxLayout *verticalLayout_4;
    QLabel *lbRnNumPt;
    QLineEdit *ledRnNumPt;
    QLabel *lbRnNormVec;
    QWidget *wRnNormVec;
    QFormLayout *formLayout;
    QLabel *lbRnNormVecY;
    QLineEdit *ledRnNormVecY;
    QLabel *lbRnNormVecZ;
    QLineEdit *ledRnNormVecX;
    QLineEdit *ledRnNormVecZ;
    QLabel *lbRnNormVecX;
    QLabel *lbRnRadius;
    QLineEdit *ledRnRadius;
    QLabel *lbRnSize;
    QLineEdit *ledRnSize;
    QLabel *lbRnColor;
    QWidget *wRnColor;
    QHBoxLayout *horizontalLayout_2;
    QLabel *lbRnColorDisplay;
    QPushButton *pbRnChooseColor;
    QSpacerItem *sRn;
    QWidget *page_4;
    QVBoxLayout *verticalLayout_6;
    renderWidget *wglRender;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuOptions;

    void setupUi(QMainWindow *winMain)
    {
        if (winMain->objectName().isEmpty())
            winMain->setObjectName(QStringLiteral("winMain"));
        winMain->setWindowModality(Qt::ApplicationModal);
        winMain->resize(1149, 596);
        winMain->setStyleSheet(QLatin1String("QMainWindow {\n"
"background-color: white\n"
"}"));
        actionUser_Preferences = new QAction(winMain);
        actionUser_Preferences->setObjectName(QStringLiteral("actionUser_Preferences"));
        actionOpen = new QAction(winMain);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionSave = new QAction(winMain);
        actionSave->setObjectName(QStringLiteral("actionSave"));
        wCentral = new QWidget(winMain);
        wCentral->setObjectName(QStringLiteral("wCentral"));
        wCentral->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(wCentral->sizePolicy().hasHeightForWidth());
        wCentral->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(wCentral);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetNoConstraint);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        wRenderOpts = new QWidget(wCentral);
        wRenderOpts->setObjectName(QStringLiteral("wRenderOpts"));
        wRenderOpts->setMinimumSize(QSize(0, 0));
        wRenderOpts->setMaximumSize(QSize(250, 16777215));
        verticalLayout_7 = new QVBoxLayout(wRenderOpts);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        twRenderOpts = new QTabWidget(wRenderOpts);
        twRenderOpts->setObjectName(QStringLiteral("twRenderOpts"));
        twRenderOpts->setMaximumSize(QSize(250, 16777215));
        tabRenderOptsWorld = new QWidget();
        tabRenderOptsWorld->setObjectName(QStringLiteral("tabRenderOptsWorld"));
        verticalLayout_3 = new QVBoxLayout(tabRenderOptsWorld);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        scarOpts = new QScrollArea(tabRenderOptsWorld);
        scarOpts->setObjectName(QStringLiteral("scarOpts"));
        scarOpts->setWidgetResizable(true);
        scarwWorldOpts = new QWidget();
        scarwWorldOpts->setObjectName(QStringLiteral("scarwWorldOpts"));
        scarwWorldOpts->setGeometry(QRect(0, 0, 224, 529));
        verticalLayout_2 = new QVBoxLayout(scarwWorldOpts);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        lbRS = new QLabel(scarwWorldOpts);
        lbRS->setObjectName(QStringLiteral("lbRS"));

        verticalLayout_2->addWidget(lbRS);

        ledRS = new QLineEdit(scarwWorldOpts);
        ledRS->setObjectName(QStringLiteral("ledRS"));

        verticalLayout_2->addWidget(ledRS);

        lbBgTex = new QLabel(scarwWorldOpts);
        lbBgTex->setObjectName(QStringLiteral("lbBgTex"));

        verticalLayout_2->addWidget(lbBgTex);

        rbBgFromPreset = new QRadioButton(scarwWorldOpts);
        rbBgFromPreset->setObjectName(QStringLiteral("rbBgFromPreset"));
        rbBgFromPreset->setChecked(true);

        verticalLayout_2->addWidget(rbBgFromPreset);

        lbBgPreset1 = new QLabel(scarwWorldOpts);
        lbBgPreset1->setObjectName(QStringLiteral("lbBgPreset1"));
        lbBgPreset1->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lbBgPreset1->sizePolicy().hasHeightForWidth());
        lbBgPreset1->setSizePolicy(sizePolicy1);
        lbBgPreset1->setMinimumSize(QSize(0, 50));

        verticalLayout_2->addWidget(lbBgPreset1);

        rbBgFromFile = new QRadioButton(scarwWorldOpts);
        rbBgFromFile->setObjectName(QStringLiteral("rbBgFromFile"));

        verticalLayout_2->addWidget(rbBgFromFile);

        wBgFromFile = new QWidget(scarwWorldOpts);
        wBgFromFile->setObjectName(QStringLiteral("wBgFromFile"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(wBgFromFile->sizePolicy().hasHeightForWidth());
        wBgFromFile->setSizePolicy(sizePolicy2);
        horizontalLayout = new QHBoxLayout(wBgFromFile);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        ledBgPath = new QLineEdit(wBgFromFile);
        ledBgPath->setObjectName(QStringLiteral("ledBgPath"));

        horizontalLayout->addWidget(ledBgPath);

        pbChooseBgPath = new QPushButton(wBgFromFile);
        pbChooseBgPath->setObjectName(QStringLiteral("pbChooseBgPath"));
        pbChooseBgPath->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(pbChooseBgPath);


        verticalLayout_2->addWidget(wBgFromFile);

        spWorldOpts = new QSpacerItem(20, 322, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(spWorldOpts);

        scarOpts->setWidget(scarwWorldOpts);

        verticalLayout_3->addWidget(scarOpts);

        twRenderOpts->addTab(tabRenderOptsWorld, QString());
        tabRenderOptsObject = new QWidget();
        tabRenderOptsObject->setObjectName(QStringLiteral("tabRenderOptsObject"));
        verticalLayout = new QVBoxLayout(tabRenderOptsObject);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        stwObjectOpts = new QStackedWidget(tabRenderOptsObject);
        stwObjectOpts->setObjectName(QStringLiteral("stwObjectOpts"));
        pageRn = new QWidget();
        pageRn->setObjectName(QStringLiteral("pageRn"));
        verticalLayout_5 = new QVBoxLayout(pageRn);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(0, 0, 0, -1);
        scarRingOpts = new QScrollArea(pageRn);
        scarRingOpts->setObjectName(QStringLiteral("scarRingOpts"));
        scarRingOpts->setWidgetResizable(true);
        scarwRingOpts = new QWidget();
        scarwRingOpts->setObjectName(QStringLiteral("scarwRingOpts"));
        scarwRingOpts->setGeometry(QRect(0, 0, 224, 520));
        verticalLayout_4 = new QVBoxLayout(scarwRingOpts);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        lbRnNumPt = new QLabel(scarwRingOpts);
        lbRnNumPt->setObjectName(QStringLiteral("lbRnNumPt"));

        verticalLayout_4->addWidget(lbRnNumPt);

        ledRnNumPt = new QLineEdit(scarwRingOpts);
        ledRnNumPt->setObjectName(QStringLiteral("ledRnNumPt"));

        verticalLayout_4->addWidget(ledRnNumPt);

        lbRnNormVec = new QLabel(scarwRingOpts);
        lbRnNormVec->setObjectName(QStringLiteral("lbRnNormVec"));

        verticalLayout_4->addWidget(lbRnNormVec);

        wRnNormVec = new QWidget(scarwRingOpts);
        wRnNormVec->setObjectName(QStringLiteral("wRnNormVec"));
        formLayout = new QFormLayout(wRnNormVec);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setHorizontalSpacing(6);
        formLayout->setVerticalSpacing(0);
        formLayout->setContentsMargins(0, 0, 0, 0);
        lbRnNormVecY = new QLabel(wRnNormVec);
        lbRnNormVecY->setObjectName(QStringLiteral("lbRnNormVecY"));

        formLayout->setWidget(1, QFormLayout::LabelRole, lbRnNormVecY);

        ledRnNormVecY = new QLineEdit(wRnNormVec);
        ledRnNormVecY->setObjectName(QStringLiteral("ledRnNormVecY"));

        formLayout->setWidget(1, QFormLayout::FieldRole, ledRnNormVecY);

        lbRnNormVecZ = new QLabel(wRnNormVec);
        lbRnNormVecZ->setObjectName(QStringLiteral("lbRnNormVecZ"));

        formLayout->setWidget(2, QFormLayout::LabelRole, lbRnNormVecZ);

        ledRnNormVecX = new QLineEdit(wRnNormVec);
        ledRnNormVecX->setObjectName(QStringLiteral("ledRnNormVecX"));

        formLayout->setWidget(0, QFormLayout::FieldRole, ledRnNormVecX);

        ledRnNormVecZ = new QLineEdit(wRnNormVec);
        ledRnNormVecZ->setObjectName(QStringLiteral("ledRnNormVecZ"));

        formLayout->setWidget(2, QFormLayout::FieldRole, ledRnNormVecZ);

        lbRnNormVecX = new QLabel(wRnNormVec);
        lbRnNormVecX->setObjectName(QStringLiteral("lbRnNormVecX"));

        formLayout->setWidget(0, QFormLayout::LabelRole, lbRnNormVecX);


        verticalLayout_4->addWidget(wRnNormVec);

        lbRnRadius = new QLabel(scarwRingOpts);
        lbRnRadius->setObjectName(QStringLiteral("lbRnRadius"));

        verticalLayout_4->addWidget(lbRnRadius);

        ledRnRadius = new QLineEdit(scarwRingOpts);
        ledRnRadius->setObjectName(QStringLiteral("ledRnRadius"));

        verticalLayout_4->addWidget(ledRnRadius);

        lbRnSize = new QLabel(scarwRingOpts);
        lbRnSize->setObjectName(QStringLiteral("lbRnSize"));

        verticalLayout_4->addWidget(lbRnSize);

        ledRnSize = new QLineEdit(scarwRingOpts);
        ledRnSize->setObjectName(QStringLiteral("ledRnSize"));

        verticalLayout_4->addWidget(ledRnSize);

        lbRnColor = new QLabel(scarwRingOpts);
        lbRnColor->setObjectName(QStringLiteral("lbRnColor"));

        verticalLayout_4->addWidget(lbRnColor);

        wRnColor = new QWidget(scarwRingOpts);
        wRnColor->setObjectName(QStringLiteral("wRnColor"));
        horizontalLayout_2 = new QHBoxLayout(wRnColor);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        lbRnColorDisplay = new QLabel(wRnColor);
        lbRnColorDisplay->setObjectName(QStringLiteral("lbRnColorDisplay"));
        lbRnColorDisplay->setStyleSheet(QLatin1String("QLabel {\n"
"	background-color : red;\n"
"	border: 1px solid gray\n"
"}"));

        horizontalLayout_2->addWidget(lbRnColorDisplay);

        pbRnChooseColor = new QPushButton(wRnColor);
        pbRnChooseColor->setObjectName(QStringLiteral("pbRnChooseColor"));
        pbRnChooseColor->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_2->addWidget(pbRnChooseColor);


        verticalLayout_4->addWidget(wRnColor);

        sRn = new QSpacerItem(173, 213, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(sRn);

        scarRingOpts->setWidget(scarwRingOpts);

        verticalLayout_5->addWidget(scarRingOpts);

        stwObjectOpts->addWidget(pageRn);
        page_4 = new QWidget();
        page_4->setObjectName(QStringLiteral("page_4"));
        verticalLayout_6 = new QVBoxLayout(page_4);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        stwObjectOpts->addWidget(page_4);

        verticalLayout->addWidget(stwObjectOpts);

        twRenderOpts->addTab(tabRenderOptsObject, QString());

        verticalLayout_7->addWidget(twRenderOpts);


        gridLayout->addWidget(wRenderOpts, 2, 1, 1, 1);

        wglRender = new renderWidget(wCentral);
        wglRender->setObjectName(QStringLiteral("wglRender"));

        gridLayout->addWidget(wglRender, 2, 0, 1, 1);

        winMain->setCentralWidget(wCentral);
        menubar = new QMenuBar(winMain);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 1149, 21));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuOptions = new QMenu(menubar);
        menuOptions->setObjectName(QStringLiteral("menuOptions"));
        winMain->setMenuBar(menubar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuOptions->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuOptions->addAction(actionUser_Preferences);

        retranslateUi(winMain);

        twRenderOpts->setCurrentIndex(0);
        stwObjectOpts->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(winMain);
    } // setupUi

    void retranslateUi(QMainWindow *winMain)
    {
        winMain->setWindowTitle(QApplication::translate("winMain", "MainWindow", nullptr));
        actionUser_Preferences->setText(QApplication::translate("winMain", "User Preferences", nullptr));
        actionOpen->setText(QApplication::translate("winMain", "Open...", nullptr));
        actionSave->setText(QApplication::translate("winMain", "Save...", nullptr));
        lbRS->setText(QApplication::translate("winMain", "Schwarzschild radius", nullptr));
        lbBgTex->setText(QApplication::translate("winMain", "Background texture", nullptr));
        rbBgFromPreset->setText(QApplication::translate("winMain", "From preset", nullptr));
        rbBgFromFile->setText(QApplication::translate("winMain", "From file", nullptr));
        pbChooseBgPath->setText(QApplication::translate("winMain", "...", nullptr));
        twRenderOpts->setTabText(twRenderOpts->indexOf(tabRenderOptsWorld), QApplication::translate("winMain", "World", nullptr));
        lbRnNumPt->setText(QApplication::translate("winMain", "Number of particles", nullptr));
        lbRnNormVec->setText(QApplication::translate("winMain", "Normal vector", nullptr));
        lbRnNormVecY->setText(QApplication::translate("winMain", "y", nullptr));
        lbRnNormVecZ->setText(QApplication::translate("winMain", "z", nullptr));
        lbRnNormVecX->setText(QApplication::translate("winMain", "x", nullptr));
        lbRnRadius->setText(QApplication::translate("winMain", "Radius", nullptr));
        lbRnSize->setText(QApplication::translate("winMain", "Size", nullptr));
        lbRnColor->setText(QApplication::translate("winMain", "Color", nullptr));
        lbRnColorDisplay->setText(QString());
        pbRnChooseColor->setText(QApplication::translate("winMain", "...", nullptr));
        twRenderOpts->setTabText(twRenderOpts->indexOf(tabRenderOptsObject), QApplication::translate("winMain", "Object", nullptr));
        menuFile->setTitle(QApplication::translate("winMain", "File", nullptr));
        menuOptions->setTitle(QApplication::translate("winMain", "Options", nullptr));
    } // retranslateUi

};

namespace Ui {
    class winMain: public Ui_winMain {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
