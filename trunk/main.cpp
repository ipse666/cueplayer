#ifdef LIBNOTIFY
#include <libnotify/notify.h>
#endif

#include <QApplication>
#include <QTranslator>
#include "cueplayer.h"
#include "singleclient.h"

#define APPVERSION "0.25"

int main(int argc, char *argv[])
{
	bool tray;
	QDir currentdir;
	QRegExp rxPath("^/.*");
	QRegExp rxFilename3("^(mms://|http://|ftp://).*");
	QString fullver;

	QString arg = QObject::trUtf8(argv[1]);
	QApplication app(argc, argv);
	SingleServer *sserver = new SingleServer;
	SingleClient *sclient = new SingleClient;
	sclient->connServer("cueplayer");
	if (sclient->getState() == "ConnectedState")
	{
		if (argc >= 2)
			sclient->setArgs(QByteArray(argv[1]));
		return 0;
	}
	else if (sclient->getState() == "UnconnectedState")
	{
		sserver->startServer();
	}
	else
	{
		qDebug() << QObject::trUtf8("Ошибка. Отпишитесь разработчику.");
		return 1;
	}

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		/*QMessageBox::critical(0, QObject::trUtf8("Лоток"),
		QObject::trUtf8("Не найден системный лоток в вашей системе."
						"<p>Вероятна некорректная работа приложения."
						"<p>Работа приложения будет продолжена."));
		return 1;*/
		tray = false;
	}
	else
	{
		QApplication::setQuitOnLastWindowClosed(false);
		tray = true;
	}
	QCoreApplication::setOrganizationName("CuePlayer");
	QTranslator myappTranslator;
		 myappTranslator.load("/usr/share/cueplayer/loc/cueplayer_" + QLocale::system().name().left(2));
		 app.installTranslator(&myappTranslator);
	fullver = APPVERSION;
#ifdef BUILDER
#ifdef DATAVER
	fullver += "-r";
	fullver += REVISION;
#endif
#endif
	app.setApplicationName("cueplayer");
	app.setApplicationVersion(fullver);
	gst_init(0,0);
	gst_registry_fork_set_enabled(true);
#ifdef LIBNOTIFY
	/* Init libnotify library */
	notify_init("CuePlayer");
#endif

	CuePlayer *player = new CuePlayer;
	player->setTray(tray);
	player->setServer(sserver);
	player->show();
	if(argc >= 2)
	{
		QStringList file;
		QString fullfile = currentdir.currentPath() + "/" + arg;
		if (rxPath.indexIn(arg) != -1 ||
			rxFilename3.indexIn(arg) != -1)
			file << arg;
		else
			file << fullfile;
		player->paramFile(file);
	}
	return app.exec();
}
