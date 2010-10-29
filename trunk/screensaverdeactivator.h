#ifndef SCREENSAVERDEACTIVATOR_H
#define SCREENSAVERDEACTIVATOR_H

#include <QObject>
#include <QHash>
#include <QProcess>
#include <QTimer>

class ScreenSaverDeactivator : public QObject
{
    Q_OBJECT
public:
    explicit ScreenSaverDeactivator(QObject *parent = 0);
    QHash<QString, bool> screenSaverCheck();
    void xDeactivate();
    void gDeactivate();
    void xActivate();
    void gActivate();

private:
    QProcess *checksaverProcess;
    QProcess *blockXscreensaverProcess;
    QProcess *blockGnomescreensaverProcess;
    QTimer *xtimer;

private slots:
    void timerUpdate();

signals:

public slots:

};

#endif // SCREENSAVERDEACTIVATOR_H
