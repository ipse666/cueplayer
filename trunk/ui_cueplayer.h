/********************************************************************************
** Form generated from reading ui file 'cueplayer.ui'
**
** Created: Mon Jul 6 03:14:16 2009
**      by: Qt User Interface Compiler version 4.5.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CUEPLAYER_H
#define UI_CUEPLAYER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CuePlayer
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QSlider *timeLineSlider;
    QLCDNumber *minNumLCD;
    QLabel *labelRazd;
    QLCDNumber *secNumLCD;
    QHBoxLayout *horizontalLayout;
    QPushButton *openButton;
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    QPushButton *prewButton;
    QPushButton *nextButton;
    QPushButton *fileList;
    QDial *volumeDial;
    QTreeWidget *treeWidget;

    void setupUi(QWidget *CuePlayer)
    {
        if (CuePlayer->objectName().isEmpty())
            CuePlayer->setObjectName(QString::fromUtf8("CuePlayer"));
        CuePlayer->resize(531, 204);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CuePlayer->sizePolicy().hasHeightForWidth());
        CuePlayer->setSizePolicy(sizePolicy);
        CuePlayer->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/knotify.png")), QIcon::Normal, QIcon::Off);
        CuePlayer->setWindowIcon(icon);
        verticalLayout_3 = new QVBoxLayout(CuePlayer);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(CuePlayer);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        timeLineSlider = new QSlider(CuePlayer);
        timeLineSlider->setObjectName(QString::fromUtf8("timeLineSlider"));
        timeLineSlider->setFocusPolicy(Qt::TabFocus);
        timeLineSlider->setMaximum(300);
        timeLineSlider->setTracking(true);
        timeLineSlider->setOrientation(Qt::Horizontal);
        timeLineSlider->setTickPosition(QSlider::TicksAbove);
        timeLineSlider->setTickInterval(0);

        verticalLayout->addWidget(timeLineSlider);


        horizontalLayout_2->addLayout(verticalLayout);

        minNumLCD = new QLCDNumber(CuePlayer);
        minNumLCD->setObjectName(QString::fromUtf8("minNumLCD"));
        minNumLCD->setFrameShape(QFrame::NoFrame);
        minNumLCD->setFrameShadow(QFrame::Raised);
        minNumLCD->setSmallDecimalPoint(false);
        minNumLCD->setNumDigits(2);
        minNumLCD->setSegmentStyle(QLCDNumber::Flat);
        minNumLCD->setProperty("value", QVariant(0));

        horizontalLayout_2->addWidget(minNumLCD);

        labelRazd = new QLabel(CuePlayer);
        labelRazd->setObjectName(QString::fromUtf8("labelRazd"));
        QFont font;
        font.setPointSize(26);
        labelRazd->setFont(font);

        horizontalLayout_2->addWidget(labelRazd);

        secNumLCD = new QLCDNumber(CuePlayer);
        secNumLCD->setObjectName(QString::fromUtf8("secNumLCD"));
        secNumLCD->setFrameShape(QFrame::NoFrame);
        secNumLCD->setFrameShadow(QFrame::Raised);
        secNumLCD->setLineWidth(1);
        secNumLCD->setSmallDecimalPoint(false);
        secNumLCD->setNumDigits(2);
        secNumLCD->setMode(QLCDNumber::Dec);
        secNumLCD->setSegmentStyle(QLCDNumber::Flat);
        secNumLCD->setProperty("value", QVariant(0));

        horizontalLayout_2->addWidget(secNumLCD);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        openButton = new QPushButton(CuePlayer);
        openButton->setObjectName(QString::fromUtf8("openButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(openButton->sizePolicy().hasHeightForWidth());
        openButton->setSizePolicy(sizePolicy1);
        openButton->setMinimumSize(QSize(45, 0));
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(":/images/images/fileopen.png")), QIcon::Normal, QIcon::Off);
        openButton->setIcon(icon1);
        openButton->setAutoDefault(false);

        horizontalLayout->addWidget(openButton);

        playButton = new QPushButton(CuePlayer);
        playButton->setObjectName(QString::fromUtf8("playButton"));
        sizePolicy1.setHeightForWidth(playButton->sizePolicy().hasHeightForWidth());
        playButton->setSizePolicy(sizePolicy1);
        playButton->setMinimumSize(QSize(45, 0));
        QIcon icon2;
        icon2.addPixmap(QPixmap(QString::fromUtf8(":/images/images/play.png")), QIcon::Normal, QIcon::Off);
        playButton->setIcon(icon2);
        playButton->setAutoDefault(false);

        horizontalLayout->addWidget(playButton);

        pauseButton = new QPushButton(CuePlayer);
        pauseButton->setObjectName(QString::fromUtf8("pauseButton"));
        sizePolicy1.setHeightForWidth(pauseButton->sizePolicy().hasHeightForWidth());
        pauseButton->setSizePolicy(sizePolicy1);
        pauseButton->setMinimumSize(QSize(45, 0));
        pauseButton->setAutoFillBackground(false);
        QIcon icon3;
        icon3.addPixmap(QPixmap(QString::fromUtf8(":/images/images/pause.png")), QIcon::Normal, QIcon::Off);
        pauseButton->setIcon(icon3);

        horizontalLayout->addWidget(pauseButton);

        stopButton = new QPushButton(CuePlayer);
        stopButton->setObjectName(QString::fromUtf8("stopButton"));
        sizePolicy1.setHeightForWidth(stopButton->sizePolicy().hasHeightForWidth());
        stopButton->setSizePolicy(sizePolicy1);
        stopButton->setMinimumSize(QSize(45, 0));
        QIcon icon4;
        icon4.addPixmap(QPixmap(QString::fromUtf8(":/images/images/stop.png")), QIcon::Normal, QIcon::Off);
        stopButton->setIcon(icon4);

        horizontalLayout->addWidget(stopButton);

        prewButton = new QPushButton(CuePlayer);
        prewButton->setObjectName(QString::fromUtf8("prewButton"));
        sizePolicy1.setHeightForWidth(prewButton->sizePolicy().hasHeightForWidth());
        prewButton->setSizePolicy(sizePolicy1);
        prewButton->setMinimumSize(QSize(45, 0));
        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/images/images/prev.png")), QIcon::Normal, QIcon::Off);
        prewButton->setIcon(icon5);

        horizontalLayout->addWidget(prewButton);

        nextButton = new QPushButton(CuePlayer);
        nextButton->setObjectName(QString::fromUtf8("nextButton"));
        sizePolicy1.setHeightForWidth(nextButton->sizePolicy().hasHeightForWidth());
        nextButton->setSizePolicy(sizePolicy1);
        nextButton->setMinimumSize(QSize(45, 0));
        QIcon icon6;
        icon6.addPixmap(QPixmap(QString::fromUtf8(":/images/images/next.png")), QIcon::Normal, QIcon::Off);
        nextButton->setIcon(icon6);

        horizontalLayout->addWidget(nextButton);

        fileList = new QPushButton(CuePlayer);
        fileList->setObjectName(QString::fromUtf8("fileList"));
        sizePolicy1.setHeightForWidth(fileList->sizePolicy().hasHeightForWidth());
        fileList->setSizePolicy(sizePolicy1);
        fileList->setMinimumSize(QSize(45, 0));
        QIcon icon7;
        icon7.addPixmap(QPixmap(QString::fromUtf8(":/images/images/format-list-unordered.png")), QIcon::Normal, QIcon::Off);
        fileList->setIcon(icon7);
        fileList->setCheckable(true);

        horizontalLayout->addWidget(fileList);


        verticalLayout_2->addLayout(horizontalLayout);


        horizontalLayout_3->addLayout(verticalLayout_2);

        volumeDial = new QDial(CuePlayer);
        volumeDial->setObjectName(QString::fromUtf8("volumeDial"));
        volumeDial->setFocusPolicy(Qt::TabFocus);
        volumeDial->setMaximum(100);
        volumeDial->setValue(100);
        volumeDial->setOrientation(Qt::Horizontal);
        volumeDial->setWrapping(false);
        volumeDial->setNotchesVisible(true);

        horizontalLayout_3->addWidget(volumeDial);


        verticalLayout_3->addLayout(horizontalLayout_3);

        treeWidget = new QTreeWidget(CuePlayer);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        treeWidget->setFocusPolicy(Qt::NoFocus);
        treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeWidget->setTextElideMode(Qt::ElideRight);
        treeWidget->setRootIsDecorated(false);
        treeWidget->setColumnCount(2);

        verticalLayout_3->addWidget(treeWidget);

        QWidget::setTabOrder(openButton, playButton);
        QWidget::setTabOrder(playButton, pauseButton);
        QWidget::setTabOrder(pauseButton, stopButton);
        QWidget::setTabOrder(stopButton, prewButton);
        QWidget::setTabOrder(prewButton, nextButton);
        QWidget::setTabOrder(nextButton, fileList);
        QWidget::setTabOrder(fileList, timeLineSlider);
        QWidget::setTabOrder(timeLineSlider, volumeDial);

        retranslateUi(CuePlayer);
        QObject::connect(fileList, SIGNAL(toggled(bool)), treeWidget, SLOT(setVisible(bool)));

        QMetaObject::connectSlotsByName(CuePlayer);
    } // setupUi

    void retranslateUi(QWidget *CuePlayer)
    {
        CuePlayer->setWindowTitle(QApplication::translate("CuePlayer", "CUE \320\237\321\200\320\276\320\270\320\263\321\200\321\213\320\262\320\260\321\202\320\265\320\273\321\214", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        CuePlayer->setToolTip(QApplication::translate("CuePlayer", "\320\237\321\200\320\276\320\270\320\263\321\200\321\213\320\262\320\260\321\202\320\265\320\273\321\214 CUE \320\276\320\261\321\200\320\260\320\267\320\276\320\262", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label->setText(QString());
#ifndef QT_NO_TOOLTIP
        minNumLCD->setToolTip(QApplication::translate("CuePlayer", "\320\274\320\270\320\275\321\203\321\202\321\213", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        labelRazd->setText(QApplication::translate("CuePlayer", ":", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        secNumLCD->setToolTip(QApplication::translate("CuePlayer", "\321\201\320\265\320\272\321\203\320\275\320\264\321\213", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        openButton->setToolTip(QApplication::translate("CuePlayer", "\320\236\321\202\320\272\321\200\321\213\321\202\321\214 \321\204\320\260\320\271\320\273", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        openButton->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        openButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        playButton->setToolTip(QApplication::translate("CuePlayer", "\320\230\320\263\321\200\320\260\321\202\321\214", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        playButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        pauseButton->setToolTip(QApplication::translate("CuePlayer", "\320\237\320\260\321\203\320\267\320\260", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pauseButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        stopButton->setToolTip(QApplication::translate("CuePlayer", "\320\241\321\202\320\276\320\277", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        stopButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        prewButton->setToolTip(QApplication::translate("CuePlayer", "\320\237\321\200\320\265\320\264\321\213\320\264\321\203\321\211\320\260\321\217 \320\272\320\276\320\274\320\277\320\276\320\267\320\270\321\206\320\270\321\217", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        prewButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        nextButton->setToolTip(QApplication::translate("CuePlayer", "\320\241\320\273\320\265\320\264\321\203\321\216\321\211\320\260\321\217 \320\272\320\276\320\274\320\277\320\276\320\267\320\270\321\206\320\270\321\217", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        nextButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        fileList->setToolTip(QApplication::translate("CuePlayer", "\320\241\320\277\320\270\321\201\320\276\320\272 \320\262\320\276\321\201\320\277\321\200\320\276\320\270\320\267\320\262\320\265\320\264\320\265\320\275\320\270\321\217", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        fileList->setText(QString());
#ifndef QT_NO_TOOLTIP
        volumeDial->setToolTip(QApplication::translate("CuePlayer", "\320\263\321\200\320\276\320\274\320\272\320\276\321\201\321\202\321\214", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
        ___qtreewidgetitem->setText(1, QApplication::translate("CuePlayer", "2", 0, QApplication::UnicodeUTF8));
        ___qtreewidgetitem->setText(0, QApplication::translate("CuePlayer", "1", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(CuePlayer);
    } // retranslateUi

};

namespace Ui {
    class CuePlayer: public Ui_CuePlayer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CUEPLAYER_H
