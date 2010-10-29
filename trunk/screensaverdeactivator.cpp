#include "screensaverdeactivator.h"

ScreenSaverDeactivator::ScreenSaverDeactivator(QObject *parent) :
    QObject(parent)
{
    checksaverProcess = new QProcess(this);
    blockXscreensaverProcess = new QProcess(this);
    blockGnomescreensaverProcess = new QProcess(this);
    xtimer = new QTimer(this);

    connect(xtimer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
}

QHash<QString, bool> ScreenSaverDeactivator::screenSaverCheck()
{
    QHash<QString, bool> screensavers;
    QByteArray result;

    if (checksaverProcess->state())
            checksaverProcess->kill();
    checksaverProcess->start("xscreensaver-command", QStringList() << "-version");
    checksaverProcess->waitForFinished(1000);
    result = checksaverProcess->readAllStandardOutput();
    checksaverProcess->close();
    screensavers.insert("xscreensaver", (result.indexOf("XScreenSaver") != -1));

    if (checksaverProcess->state())
            checksaverProcess->kill();
    checksaverProcess->start("gnome-screensaver-command", QStringList() << "--version");
    checksaverProcess->waitForFinished(1000);
    result = checksaverProcess->readAll();
    checksaverProcess->close();
    screensavers.insert("gnome-screensaver", (result.indexOf("gnome-screensaver-command") != -1));

    return screensavers;
}

void ScreenSaverDeactivator::xDeactivate()
{
    xtimer->start(500);
}

void ScreenSaverDeactivator::xActivate()
{
    xtimer->stop();
}

void ScreenSaverDeactivator::gActivate()
{
    blockGnomescreensaverProcess->close();
}

void ScreenSaverDeactivator::gDeactivate()
{
    if (blockGnomescreensaverProcess->state())
        blockGnomescreensaverProcess->kill();
    blockGnomescreensaverProcess->start("gnome-screensaver-command", QStringList() << "-i");
}

void ScreenSaverDeactivator::timerUpdate()
{
    if (blockXscreensaverProcess->state())
        blockXscreensaverProcess->kill();
    blockXscreensaverProcess->start("xscreensaver-command", QStringList() << "-deactivate");
}
